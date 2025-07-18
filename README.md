# cluster-osc-heart-rate
cluster OSC機能を使って ワールド内で 心拍線センサーの値を表示したり利用したりするためのサンプルです。


## 概要
心拍センサーデバイス  
[README](./M5StickC/osc-heart-rate/README.md)
- M5StickC Plus2 
- GROVE - 心拍センサ  

Unity(Cluster Creator Kit Script)  
[README](./Unity/README.md)
- cluster-osc-heart-rate.unitypackage  

# サンプル設置 cluster ワールド
[OSC 心拍線センサー サンプル](https://cluster.mu/w/1f5c2d7d-23d3-45f3-b7c3-3f770c4ca261)  
心拍センサーデバイス があれば表示を試すことが出来ます  
1. cluster アプリを起動
2. 設定画面 -> OSC 受信を有効にする
3. IPアドレス, ポート を確認する
4. 心拍センサーデバイス に電源を入れて `M% ボタン` を押す
5. スマートフォンなどで Wi-Fi AP `OSC App` に接続する
6. SSID,Password や OSC デバイス名, 送信先 IPアドレス, ポートを設定する
7. 耳たぶにセンサーを装着する
8. cluster ワールドで 心拍が表示されることを確認する

