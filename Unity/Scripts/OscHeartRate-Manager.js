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