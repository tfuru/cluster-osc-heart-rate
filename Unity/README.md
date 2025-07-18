# cluster-osc-heart-rate
cluster OSC機能を使って ワールド内で 心拍線センサーの値を表示したり利用したりするためのサンプルです。

# 概要

# unitypackage
[OscHeartRate.unitypackage](./OscHeartRate.unitypackage)

# サンプルシーンの開き方
1. ClusterCreatorKitTemplate をダウンロードする
2. フォルダ名を変更
    - `ClusterCreatorKitTemplate` → `OscHeartRateSample` など
3. Unity を起動し、OscHeartRateSample を開く
4. `OscHeartRate.unitypackage` をインポートする
5. `Assets/t_furu/OscHeartRate/Scenes/osc-heart-rate.unity` を開く

# Scripts

```OscHeartRate-Manager.js

/** OscHeartRate-Manager.js
 * 
 * 入室したプレイヤーに OscHeartRate-Player.js を適用する
 * 
 * Cluster Creator Kit Script Reference
 * https://docs.cluster.mu/script/
 * 
 */

// プレイヤー検索間隔
const PLAYER_SEARCH_INTERVAL = 1; // 秒

const MODE = {
  INITIALIZE: 'INITIALIZE',
}

const setMode = (mode) => {
  switch (mode) {
    case MODE.INITIALIZE:
      initialize();
      break;
    default:
      _.log(`Unknown mode: ${mode}`);
      return;
  }
  $.state.mode = mode;  
}

// 初期化処理
const initialize = () => {
  $.state.players = [];
}

const onUpdateSetPlayerScript = (deltaTime) => {
  let t = $.state.timer ?? 0;
  t += deltaTime;
  $.state.timer = t;
  if (t < PLAYER_SEARCH_INTERVAL) return;  
  $.state.timer = 0;

  const players = $.state.players ?? [];
  // 全てのユーザーを検索して、Player スクリプトを適用する
  const nearbyPlayers = $.getPlayersNear(new Vector3(0,0,0), Infinity);
  // $.state.players id と nearbyPlayers id を比較して、差分を取得する
  const newPlayers = nearbyPlayers.filter(player => !players.some(p => p.id === player.id));
  if (newPlayers.length === 0) {
    // $.log(`No new players found.`);
    return;
  }
  $.log(`New players found: ${newPlayers.length}`);
  newPlayers.forEach(player => {
    if (player.exists() === false) {
      return;
    }
    $.setPlayerScript(player);
    player.send("INITIALIZE",{});
    players.push(player);
  });
  $.state.players = players;
}

$.onStart(() => {
  setMode(MODE.INITIALIZE);
});

$.onUpdate(deltaTime => {
  // 全てのユーザーを検索して、Player スクリプトを適用する
  onUpdateSetPlayerScript(deltaTime);
});

$.onReceive((messageType, arg, sender) => {
  // $.log(`Item onReceive: messageType=${messageType}, arg=${arg}, sender=${sender}`);
  switch (messageType) {
    default:
      $.log(`Unknown messageType: ${messageType}`);
  }
},{item: true, player: true});
```

```OscHeartRate-Player.js
/** OscHeartRate-Player.js  
 * 
 * Cluster Creator Kit Script Reference
 * https://docs.cluster.mu/script/
 * 
 */

const MODE = {
  INITIALIZE: 'INITIALIZE',
}

const setMode = (mode) => {
  switch (mode) {
    case MODE.INITIALIZE:
      initialize();
      break;
    default:
      _.log(`Unknown mode: ${mode}`);
      return;
  }
}

// 初期化処理
const initialize = () => {
  _.log(`initialize`);
}

const onReceiveHeartRate = (timestamp, values) => {
// /avatar/parameters/HeartRate timestamp deviceName heartRate
  if (values.length < 3) {
    _.log(`Invalid HeartRate message: ${values}`);
    return;
  }
  const timestamp = values[0].getInt();
  const deviceName = values[1].getAsciiString();
  const heartRate = values[2].getInt();

  _.log(`HeartRate ${deviceName} ${heartRate}`);

  // ローカルUI の HeartRate に値をセットする
  const textComponent = _.playerLocalObject("HeartRate")?.getUnityComponent("Text");
  if (textComponent === undefined) {
    _.log(`HeartRate text component not found.`);
    return;
  }
  textComponent.unityProp.text = `${heartRate}`;
}

_.onReceive((messageType, arg, sender) => {
  // _.log(`Received message: ${messageType}, ${arg}`);
  switch (messageType) {
    case 'INITIALIZE':
      _manager = sender;
      setMode(MODE.INITIALIZE);
      break;
    default:
      _.log(`Unknown message type: ${messageType}`);
      return;
  }
},{ player: false, item: true });

_.oscHandle.onReceive(messages => {
  const lines = [];

  messages.forEach((message, i) => {
    const { address, timestamp, values } = message;

    switch(address) {
      case '/avatar/parameters/HeartRate':
        // Handle empty address if needed
        onReceiveHeartRate(timestamp, values);
        break;
      default:
        lines.push(`== message [${i + 1}/${messages.length}]`);
        lines.push(`address: ${address}`);
        lines.push(`timestamp: ${new Date(timestamp).toLocaleString()}`);

        values.forEach((value, j) => {
          lines.push(`= value [${j + 1}/${values.length}]`);

          lines.push(`getInt(): ${value.getInt()}`);
          lines.push(`getFloat(): ${value.getFloat()}`);
          lines.push(`getAsciiString(): ${value.getAsciiString()}`);
          lines.push(`getBlobAsUint8Array(): ${value.getBlobAsUint8Array()}`);
          lines.push(`getBlobAsUtf8String(): ${value.getBlobAsUtf8String()}`);
          lines.push(`getBool(): ${value.getBool()}`);
        });
        break;
    }
  });

  _.log(lines.join("\n"));
});
```