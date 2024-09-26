import * as THREE from "three";
import { OrbitControls } from "three/addons/controls/OrbitControls.js";
import Stats from "three/addons/libs/stats.module.js";

// https://dev.xingway.com/threejs-particle-system-part2/

// 创建场景
const scene = new THREE.Scene();

// 创建相机
const camera = new THREE.PerspectiveCamera(
    75,
    window.innerWidth / window.innerHeight,
    0.1,
    1000
);
camera.position.set(12, 8, 8);
camera.lookAt(0, 0, 0);

// 创建渲染器
const renderer = new THREE.WebGLRenderer();
renderer.setSize(window.innerWidth, window.innerHeight);
document.body.appendChild(renderer.domElement);

// 添加环境光
const ambientLight = new THREE.AmbientLight(0x888888);
scene.add(ambientLight);

// 添加点光源
const pointLight = new THREE.PointLight(0xffffff, 1, 100);
pointLight.position.set(10, 10, 10);
scene.add(pointLight);

let controls, stats;

controls = new OrbitControls(camera, renderer.domElement);
controls.target.set(0, 10, 0);
controls.minDistance = 25;
controls.maxDistance = 15000;
controls.maxPolarAngle = Math.PI / 1.7;
controls.autoRotate = false;
controls.autoRotateSpeed = -1;
controls.update();

stats = new Stats();
document.body.appendChild(stats.dom);

const vertexShader = `
  uniform float u_time;  //时间累计
  uniform vec3 u_gravity; //粒子重力加速度
  attribute vec3 velocity; //粒子速度
  void main() {
    vec3 vel = velocity * u_time; //根据时间计算速度
    vel = vel + u_gravity * u_time * u_time; //根据时间计算加速度
    vec3 pos = position + vel;  //计算位置偏移
    gl_Position = projectionMatrix * modelViewMatrix * vec4(pos, 1.0);
    gl_PointSize = 6.0;
  }
`;
const fragmentShader = `
  void main() {
    vec3 color = vec3(1.0);
    gl_FragColor = vec4(color, 1.0);
  }
`;

const uniforms = {
    u_time: { value: 0 },
    u_gravity: { value: new THREE.Vector3(0, -5, 0) },
};

_initParticles();
function _initParticles() {
    const material = new THREE.ShaderMaterial({
        uniforms: uniforms,
        vertexShader,
        fragmentShader,
    });
    const COUNT = 100;
    const positions = new Float32Array(COUNT * 3);
    const velocity = new Float32Array(COUNT * 3);

    const size = 0;
    const speed = 20;
    for (let i = 0; i < positions.length; i++) {
        positions[i] = (Math.random() - 0.5) * size;
        console.log(positions[i]);
        velocity[i] = (Math.random() - 0.5) * speed;
    }

    const geometry = new THREE.BufferGeometry();
    geometry.setAttribute("position", new THREE.BufferAttribute(positions, 3));
    geometry.setAttribute("velocity", new THREE.BufferAttribute(velocity, 3));
    const mesh = new THREE.Points(geometry, material);
    scene.add(mesh);
}

const clock = new THREE.Clock();

const animate = () => {
    controls.update();
    requestAnimationFrame(animate);

    uniforms.u_time.value += clock.getDelta(); //更新时间参数
    if (uniforms.u_time.value > 3.0) {
        uniforms.u_time.value = 0;
    }

    renderer.render(scene, camera);

    stats.update();
};

animate();
