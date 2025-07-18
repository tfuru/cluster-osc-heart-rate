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