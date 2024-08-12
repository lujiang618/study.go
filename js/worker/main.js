// 创建 worker
const myWorker = new Worker('myWorker.js');

const m = [];

// 监听 myWorker 将缓冲区传回 main
myWorker.addEventListener('message', function handleMessageFromWorker(msg) {
  //   console.log("message from worker received in main:", msg);

  const bufTransferredBackFromWorker = msg.data;
  m.push(bufTransferredBackFromWorker);
  //   console.log(
  //     "buf.byteLength in main AFTER transfer back from worker:",
  //     bufTransferredBackFromWorker.byteLength,
  //   );
});

// 创建 buffer
const myBuf = new ArrayBuffer(8);

// console.log(
//   "buf.byteLength in main BEFORE transfer to worker:",
//   myBuf.byteLength,
// );

const a = []

function p() {
    for (let i = 0; i < 300; i++) {
        myWorker.postMessage(a);
    }
 
}

function lp() {
    setInterval(() => {
        const a = m.pop()
        myWorker.postMessage(a, [a]);
    }, 10);
}



// 发送 myBuf 给 myWorker 并转移底层 ArrayBuffer
myWorker.postMessage(myBuf, [myBuf]);

// console.log(
//   "buf.byteLength in main AFTER transfer to worker:",
//   myBuf.byteLength,
// );
