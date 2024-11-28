package main

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"encoding/hex"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"sync"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
)

// Mutex to handle concurrent access
var mu sync.Mutex

// 密钥（可以替换成更强的密钥）
const encryptionKey = "0123456789abcdef" // 16字节密钥

// 文件路径用于保存文档
const filePath = "document.txt"

// 用于WebSocket连接管理
var clients = make(map[*websocket.Conn]bool)
var broadcast = make(chan string) // 广播消息的通道

// 加密函数
func encrypt(plaintext string, key string) (string, error) {
	block, err := aes.NewCipher([]byte(key))
	if err != nil {
		return "", err
	}

	iv := make([]byte, aes.BlockSize)
	if _, err := rand.Read(iv); err != nil {
		return "", err
	}

	stream := cipher.NewCFBEncrypter(block, iv)
	ciphertext := make([]byte, len(plaintext))
	stream.XORKeyStream(ciphertext, []byte(plaintext))

	// 将IV和密文合并为一个字符串返回
	return hex.EncodeToString(iv) + hex.EncodeToString(ciphertext), nil
}

// 解密函数
func decrypt(ciphertextHex string, key string) (string, error) {
	data, err := hex.DecodeString(ciphertextHex)
	if err != nil {
		return "", err
	}

	block, err := aes.NewCipher([]byte(key))
	if err != nil {
		return "", err
	}

	// 提取IV和密文
	iv := data[:aes.BlockSize]
	ciphertext := data[aes.BlockSize:]

	stream := cipher.NewCFBDecrypter(block, iv)
	plaintext := make([]byte, len(ciphertext))
	stream.XORKeyStream(plaintext, ciphertext)

	return string(plaintext), nil
}

// 保存文档
func saveDocument(c *gin.Context) {
	content := c.PostForm("content")
	encryptedContent, err := encrypt(content, encryptionKey)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Encryption failed"})
		return
	}

	// 锁定文件，防止并发修改
	mu.Lock()
	defer mu.Unlock()

	// Write encrypted content to the file
	err = ioutil.WriteFile(filePath, []byte(encryptedContent), 0644)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to save document"})
		return
	}

	// 广播新内容给所有连接的 WebSocket 客户端
	log.Println("Broadcasting document content:", content)
	broadcast <- content // 将新内容传给广播通道

	c.JSON(http.StatusOK, gin.H{"status": "Document saved!"})
}

// 获取文档
func getDocument(c *gin.Context) {
	// 锁定文件，防止并发读取
	mu.Lock()
	defer mu.Unlock()

	// Read the encrypted content from the file
	encryptedContent, err := ioutil.ReadFile(filePath)
	if err != nil {
		if os.IsNotExist(err) {
			c.JSON(http.StatusNotFound, gin.H{"error": "Document not found"})
		} else {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to read document"})
		}
		return
	}

	// 解密文档内容
	decryptedContent, err := decrypt(string(encryptedContent), encryptionKey)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Decryption failed"})
		return
	}

	c.JSON(http.StatusOK, gin.H{"content": decryptedContent})
}

// WebSocket 连接处理
func handleConnections(c *gin.Context) {
	upgrader := websocket.Upgrader{
		CheckOrigin: func(r *http.Request) bool {
			return true
		},
	}

	// 升级 HTTP 请求为 WebSocket
	conn, err := upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		log.Println(err)
		return
	}
	defer conn.Close()

	// 将连接添加到客户端列表
	clients[conn] = true
	log.Println("New WebSocket connection")

	// 监听广播，向所有 WebSocket 客户端发送消息
	for {
		select {
		case msg := <-broadcast:
			// 广播消息给所有 WebSocket 客户端
			log.Println("Sending broadcast message:", msg)
			for client := range clients {
				err := client.WriteMessage(websocket.TextMessage, []byte(msg))
				if err != nil {
					log.Println("Error sending message:", err)
					client.Close()
					delete(clients, client)
				}
			}
		}
	}
}

// 根路由的处理函数，返回编辑页面
func rootHandler(c *gin.Context) {
	c.HTML(http.StatusOK, "editor.html", nil) // 渲染编辑页面
}

func main() {
	r := gin.Default()

	// 提供静态文件服务
	r.Static("/static", "./static")

	// 提供模板渲染
	r.LoadHTMLFiles("./static/editor.html")

	// 根路由
	r.GET("/", rootHandler)

	// API 路由
	r.POST("/save", saveDocument)
	r.GET("/document", getDocument)

	// WebSocket 路由
	r.GET("/ws", func(c *gin.Context) {
		handleConnections(c)
	})

	// 启动服务器
	log.Fatal(r.Run("0.0.0.0:8080"))
}
