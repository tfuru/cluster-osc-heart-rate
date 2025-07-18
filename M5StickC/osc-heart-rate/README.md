# cluster-osc-heart-rate
cluster OSC機能を使って ワールド内で 心拍線センサーの値を表示したり利用したりするためのサンプルです。

- スマートフォンなどのブラウザから WiFi設定, OSC送信先の設定ができます。
- WiFi 設定を変更する場合 Aボタンを押すことで WiFi設定,OSC送信先 変更ができます。

## 概要
心拍センサーデバイス
- [M5StickC Plus2](https://www.switch-science.com/products/9420?_pos=2&_sid=c7a9ffe1e&_ss=r)
- [GROVE - 心拍センサ](https://www.switch-science.com/products/2526?variant=42381928988870)

## 開発環境
VSCode + PlatformIO
1. VSCode(Visual STudio Code)をインストール
2. PlatformIOをインストール
   - VSCodeの拡張機能から「PlatformIO IDE」を検索してインストール
3. プロジェクトディレクトリを開く
   ```
   cd cluster-osc-heart-rate/M5StickC/osc-heart-rate/
   code .
   ```

## 利用ライブラリ
`lib/setup.sh` を実行して、必要なライブラリを取得できます 

```
cd lib/
chmod +x setup.sh
./setup.sh

# git clone https://github.com/m5stack/M5StickCPlus2.git
# git clone https://github.com/m5stack/M5Unified.git
# git clone https://github.com/m5stack/M5GFX.git
# git clone https://github.com/CNMAT/OSC.git
# git clone https://github.com/tzapu/WiFiManager.git
```

## OSC 送信データ

```
/avatar/parameters/HeartRate [timestamp (ms)] [デバイス名] [心拍数]
/avatar/parameters/HeartRate 1752857438185 HeartRate 94
```
