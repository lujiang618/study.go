import * as THREE from "three";
import { OrbitControls } from "three/addons/controls/OrbitControls.js";
import Stats from "stats-gl";

// 初始化场景、相机和渲染器
const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
const renderer = new THREE.WebGLRenderer();
renderer.setSize(window.innerWidth, window.innerHeight);
document.body.appendChild(renderer.domElement);

camera.position.x = 29;
camera.position.y = 18;
camera.position.z = 50;

// 创建平面
const planeGeometry = new THREE.BoxGeometry(10, 0.01, 5);
const planeMaterial = new THREE.MeshBasicMaterial({ color: 0xff0000, side: THREE.DoubleSide });
const plane = new THREE.Mesh(planeGeometry, planeMaterial);
plane.position.y = -5;
scene.add(plane);

// 粒子着色器
const vertexShader = `
    uniform float uTime;
    attribute vec3 offset;
    attribute float velocity;
    varying float vAlpha;
    void main() {
        vec3 pos = offset;
        pos.y -= mod(uTime * velocity, 20.0);
        if (pos.y < -10.0) {
            pos.y += 20.0;
        }

        // 检测碰撞并反弹
        if (pos.y <= -5.0 && pos.y >= -5.01 && pos.x >= -10.0 && pos.x <= 10.0 && pos.z >= -5.0 && pos.z <= 5.0) {
            pos.y = -5.0 + (-5.0 - pos.y);
        }

        gl_Position = projectionMatrix * modelViewMatrix * vec4(pos, 1.0);
        gl_PointSize = 3.0;
    }
`;

const fragmentShader = `
    void main() {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
`;

// 创建粒子系统
const particleCount = 10000;
const offsets = new Float32Array(particleCount * 3);
const velocities = new Float32Array(particleCount);

for (let i = 0; i < particleCount; i++) {
    offsets[i * 3] = (Math.random() * 2 - 1) * 10; // x
    offsets[i * 3 + 1] = (Math.random() * 2 - 1) * 10; // y
    offsets[i * 3 + 2] = (Math.random() * 2 - 1) * 10; // z

    velocities[i] = Math.random() * 0.1 + 0.02; // y velocity
}

const geometry = new THREE.InstancedBufferGeometry();
geometry.setAttribute('position', new THREE.Float32BufferAttribute([0, 0, 0], 3));
geometry.setAttribute('offset', new THREE.InstancedBufferAttribute(offsets, 3));
geometry.setAttribute('velocity', new THREE.InstancedBufferAttribute(velocities, 1));

const material = new THREE.ShaderMaterial({
    vertexShader,
    fragmentShader,
    uniforms: {
        uTime: { value: 0.0 }
    },
    transparent: true
});

const particleSystem = new THREE.Points(geometry, material);
scene.add(particleSystem);

let controls, stats;

controls = new OrbitControls(camera, renderer.domElement);
controls.minDistance = 5;
controls.maxDistance = 500;
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
    material.uniforms.uTime.value = time * 0.1;
    renderer.render(scene, camera);
}

animate(0);

// 处理窗口大小变化
window.addEventListener('resize', () => {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
});
