一、环境搭建
	sudo tar -C /usr/local/ -xzf go1.17.4.tar.gz
	~/.bashrc中添加
		export GOROOT=/usr/local/go1.17.4
		export GOPATH=/usr/share/gocode
		export PATH=$GOPATH/bin:$GOROOT/bin:$PATH
	source ~/.bashrc && go env 测试环境是否搭建完成
