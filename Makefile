# よく使う操作のショートカット (中身は scripts/*.sh)
#   make help
.PHONY: help net ssh-key deploy build list run preflight log fetch docker plot dummy

help:
	@echo "make net                  Mac ⇔ G1 の接続確認"
	@echo "make ssh-key              SSH 公開鍵を内部PCに登録 (初回のみ)"
	@echo "make build                内部PCへ転送してビルド"
	@echo "make list                 内部PCのビルド済みプログラム一覧"
	@echo "make preflight            実験前チェック (疎通 + 健全性)"
	@echo "make run P=<prog> A=\"..\"  内部PCで実行 (例: make run P=g1_loco_cli)"
	@echo "make log D=10 L=label     状態を記録して Mac に取り寄せ"
	@echo "make fetch                内部PCの logs/ を同期"
	@echo "make plot F=logs/x.csv    ログを可視化"
	@echo "make docker               Mac 上でコンパイル確認 (Docker)"
	@echo "make dummy                ダミーログ生成 (可視化のお試し)"

net:       ; ./scripts/check_network.sh
ssh-key:   ; ./scripts/setup_ssh_key.sh
deploy:    ; ./scripts/deploy.sh
build:     ; ./scripts/remote_build.sh
list:      ; ./scripts/remote_list.sh
preflight: ; ./scripts/preflight.sh
run:       ; ./scripts/remote_run.sh $(P) $(A)
log:       ; ./scripts/remote_log.sh $(or $(D),10) $(or $(L),lowstate)
fetch:     ; ./scripts/fetch_logs.sh
docker:    ; ./scripts/docker_build.sh
plot:      ; uv run --project tools tools/plot_log.py $(F) $(A)
dummy:     ; uv run --project tools tools/make_dummy_log.py logs/dummy.csv
