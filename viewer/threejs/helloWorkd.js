const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(
    60,
    window.innerWidth / window.innerHeight,
    0.1,
    1000
);
const renderer = new THREE.WebGLRenderer({ antialias: true });

renderer.setSize(window.innerWidth, window.innerHeight);

renderer.setClearColor("#222222");
document.body.appendChild(renderer.domElement);

camera.position.set(5,5,5); // 设置相机的位置
camera.lookAt(new THREE.Vector3(0, 0, 0)); // 相机看向原点
console.log(camera.position);

// resize 事件
window.addEventListener("resize", () => {
    let width = window.innerWidth;
    let height = window.innerHeight;
    renderer.setSize(width, height);
    camera.aspect = width / height;
    camera.updateProjectionMatrix();
});

// 立体方
const geometry = new THREE.BoxGeometry(1, 1, 1);


const material = new THREE.MeshStandardMaterial({
    color: 0xff0051,
    flatShading: true,
    metalness: 0,
    roughness: 1,
});
const cube = new THREE.Mesh(geometry, material);
scene.add(cube);

// 维数据集
const geometry2 = new THREE.BoxGeometry(2, 2, 2);
const material2 = new THREE.MeshBasicMaterial({
    color: "#0000ff",
    wireframe: true,
    wireframeLinewidth: 2,
    transparent: true,
});
const wireframeCube = new THREE.Mesh(geometry2, material2);
scene.add(wireframeCube);


// 环境光
const ambientLight = new THREE.AmbientLight(0xffffff, 0.2);
scene.add(ambientLight);

// 点光源
const pointLight = new THREE.PointLight(0xffffff, 1);
pointLight.position.set(5, 5, 5);
scene.add(pointLight);

function animate() {
    requestAnimationFrame(animate);
    cube.rotation.x += 0.01;
    cube.rotation.y += 0.01;
    wireframeCube.rotation.x -= 0.01;
    wireframeCube.rotation.y -= 0.01;
    renderer.render(scene, camera);
}
animate();

// https://xie.infoq.cn/article/6db75bd9e2ce573c9f87e9145