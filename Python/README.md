# cluster-osc-heart-rate
cluster OSC機能を使って ワールド内で 心拍線センサーの値を表示したり利用したりするためのサンプルです。

## 概要
[心拍計アームバンド HW706](https://amzn.to/4lvoeQe) の データを受信する Pythonスクリプト

## 起動方法

```
pyenv versions
pyenv local 3.11.3

pip install -r requirements.txt
python main.py --osc_ip 127.0.0.1 --osc_port 9000
```

# OSC 送信データ

```
/avatar/parameters/HeartRate [timestamp (ms)] [デバイス名] [心拍数]
/avatar/parameters/HeartRate 1752857438185 HeartRate 94
```