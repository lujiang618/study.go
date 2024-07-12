import * as THREE from "three";
import { OrbitControls } from "three/addons/controls/OrbitControls.js";
import Stats from "stats-gl";

const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(
    75,
    window.innerWidth / window.innerHeight,
    0.1,
    10000000
);
camera.position.x = 85;
camera.position.y = 112;
camera.position.z = 194;

// 添加环境光
const ambientLight = new THREE.AmbientLight(0x888888);
scene.add(ambientLight);

// 添加点光源
const pointLight = new THREE.PointLight(0xffffff, 1, 100000);
pointLight.position.set(100, 100, 100);
scene.add(pointLight);

const renderer = new THREE.WebGLRenderer();
renderer.setSize(window.innerWidth, window.innerHeight);
document.body.appendChild(renderer.domElement);

// 粒子数量
const particleCount = 10000;

// 粒子几何体
const geometry = new THREE.InstancedBufferGeometry();
const positions = new Float32Array([
    -0.1, -0.1, 0, 0.1, -0.1, 0, 0.1, 0.1, 0, -0.1, 0.1, 0,
]);
geometry.setAttribute("position", new THREE.BufferAttribute(positions, 3));

const indices = new Uint16Array([0, 1, 2, 0, 2, 3]);
geometry.setIndex(new THREE.BufferAttribute(indices, 1));

// 粒子实例属性
const offsets = new Float32Array(particleCount * 3);
const velocities = new Float32Array(particleCount * 3);
const colors = new Float32Array(particleCount * 3);

for (let i = 0; i < particleCount; i++) {
    offsets[i * 3] = Math.random() * 150 - 1;
    offsets[i * 3 + 1] = Math.random() * 150 - 1;
    offsets[i * 3 + 2] = Math.random() * 150 - 1;

    velocities[i * 3] = Math.random() * 0.01 - 0.005;
    velocities[i * 3 + 1] = Math.random() * -0.1 - 0.005;
    velocities[i * 3 + 2] = Math.random() * 0.01 - 0.005;

    colors[i * 3] = Math.random();
    colors[i * 3 + 1] = Math.random();
    colors[i * 3 + 2] = Math.random();
}

geometry.setAttribute("offset", new THREE.InstancedBufferAttribute(offsets, 3));
geometry.setAttribute(
    "velocity",
    new THREE.InstancedBufferAttribute(velocities, 3)
);
geometry.setAttribute("color", new THREE.InstancedBufferAttribute(colors, 3));

// 粒子材质
const material = new THREE.ShaderMaterial({
    vertexShader: `
    attribute vec3 offset;
    attribute vec3 velocity;
    attribute vec3 color;
    varying vec3 vColor;
    uniform float time;
    void main() {
      float elapsedTime = time;
      vec3 newPosition = position + offset + velocity * time;

      vColor = color;
      gl_Position = projectionMatrix * modelViewMatrix * vec4(newPosition, 1.0);
    }
  `,
    fragmentShader: `
    varying vec3 vColor;
    void main() {
      gl_FragColor = vec4(vColor, 1.0);
    }
  `,
    uniforms: {
        time: { value: 0.0 },
    },
    blending: THREE.AdditiveBlending,
    transparent: true,
    opacity: 0.9,
    depthTest: true,
    depthWrite: false,
    fog: false,
});

const particleSystem = new THREE.Mesh(geometry, material);
scene.add(particleSystem);

let controls, stats;

controls = new OrbitControls(camera, renderer.domElement);
controls.target.set(0, 10, 0);
controls.minDistance = 0.1;
controls.maxDistance = 15000;
controls.maxPolarAngle = Math.PI / 1.7;
controls.autoRotate = false;
controls.autoRotateSpeed = -1;
controls.update();


stats = new Stats({
    precision: 3,
    horizontal: false,
});
stats.init(renderer);
document.body.appendChild(stats.dom);

// 动画循环
function animate(time) {
    controls.update();
    stats.update();

    // console.log(camera.position)

    requestAnimationFrame(animate);
    material.uniforms.time.value = time * 1;
    renderer.render(scene, camera);
}

animate(0);
