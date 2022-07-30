package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"strconv"
)

//TODO
//https://github.com/junhaideng/go-crawler-example  爬虫例子

 func fetchUrl(url string) string {
	// fmt.Println("Fetch Url", url)

	 // 创建请求
	 req, _ := http.NewRequest("GET", url, nil)
	 // 创建HTTP客户端
	 client := &http.Client{}
	 // 发出请求
	 resp, err := client.Do(req)
	 if err != nil {
		 fmt.Println("Http get err:", err)
	 }
	 if resp.StatusCode != 200 {
		 fmt.Println("Http status code:", resp.StatusCode)
	 }
	 // 读取HTTP响应正文
	 defer resp.Body.Close()
	 body, err := ioutil.ReadAll(resp.Body)
	 if err != nil {
		 fmt.Println("Read error", err)
	 }
	 return string(body)
 }

func main() {
	if len(os.Args) == 1{
		fmt.Println("error,please input vmid")
		return
	}

	var id string
	for _, v := range os.Args {
		//fmt.Printf("args[%v]=%v\n", i, v)
		id=v
	}

	// TODO 补充pn=
	baseurl := fmt.Sprintf("https://api.bilibili.com/x/relation/followings?vmid=%v&pn=",id)
	pageCount := 1

	for ;; {
		url := baseurl + strconv.Itoa(pageCount)
		body := fetchUrl(url)
		userMap := upInfoList(body)

		if len(userMap) == 0 {
			break
		}

		pageCount += 1
		for key, value := range userMap {
			ss := fmt.Sprintf("        <outline text=\"%s\" title=\"%s\" type=\"rss\" xmlUrl=\"http://127.0.0.1:1200/bilibili/user/video/%d\"/>", value, value, key)
			fmt.Println(ss)
		}
	}
}

//返回值 key: mid		value: uname
func upInfoList(json_str string)  map[int64]string {
	res := make( map[int64]string)
	var jsobj map[string]interface{}
	json.Unmarshal([]byte(json_str), &jsobj)

	if _, ok := jsobj["data"]; !ok {
		fmt.Println("bilbili 限制5页", json_str)
		return res
	}

	jsobj = jsobj["data"].(map[string]interface{})
	if jsobj["list"] == nil {
		return res
	}

	jsarray := jsobj["list"].([]interface{})
	for _, user := range jsarray {
		//一个用户
		user := user.(map[string]interface{})
		//fmt.Println(user)
		var mid int64
		var uname string
		for key,value := range user {
			if key == "mid" {
				mid = int64(value.(float64))
			} else if key == "uname" {
				uname = value.(string)
			}
		}
		res[mid]=uname
	}

	return res
}

