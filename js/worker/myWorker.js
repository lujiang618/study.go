// 监听 main 并将缓冲区转移到 myWorker
self.onmessage = function handleMessageFromMain(msg) {
  // console.log("message from main received in worker:", msg);


  // console.log(
  //   "buf.byteLength in worker BEFORE transfer back to main:",
  //   bufTransferredFromMain.byteLength,
  // );

  const size = 1024 * 1024 * 20;
  const myBuf = new ArrayBuffer(size);

  // 将 buf 发送回 main 并转移底层 ArrayBuffer
  self.postMessage(myBuf, [myBuf]);

  // console.log(
  //   "buf.byteLength in worker AFTER transfer back to main:",
  //   bufTransferredFromMain.byteLength,
  // );
};
