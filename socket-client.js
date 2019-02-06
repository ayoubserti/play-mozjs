const WebSocket = require('ws')
const url = 'ws://localhost:8082/debugger/'
const connection = new WebSocket(url)
const helloMsg = {
    me : 'Ayoub',
    even: 'handshake',
    now : Date.now()
}
connection.onopen = () => {
  connection.send(JSON.stringify(helloMsg) )
}

connection.onerror = (error) => {
  console.log(`WebSocket error: ${error}`)
}

connection.onmessage = (e) => {
  console.log(e.data)
}