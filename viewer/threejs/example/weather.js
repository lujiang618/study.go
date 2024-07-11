import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import Stats from 'three/addons/libs/stats.module.js';

// 创建场景
const scene = new THREE.Scene();

// 创建相机
const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
camera.position.z = 50;

// 创建渲染器
const renderer = new THREE.WebGLRenderer();
renderer.setSize(window.innerWidth, window.innerHeight);
document.body.appendChild(renderer.domElement);

// 添加环境光
const ambientLight = new THREE.AmbientLight(0x888888);
scene.add(ambientLight);

// 添加点光源
const pointLight = new THREE.PointLight(0xFFFFFF, 1, 100);
pointLight.position.set(10, 10, 10);
scene.add(pointLight);

let controls, stats;

// 创建粒子几何体和材质
const createParticles = (count, size, color) => {
    const particles = new THREE.BufferGeometry();
    const positions = new Float32Array(count * 3);
    const velocities = new Float32Array(count * 3);

    for (let i = 0; i < count; i++) {
        positions[i * 3] = (Math.random() - 0.5) * 200;
        positions[i * 3 + 1] = (Math.random() - 0.5) * 200;
        positions[i * 3 + 2] = (Math.random() - 0.5) * 200;

        velocities[i * 3] = 0;
        velocities[i * 3 + 1] = -Math.random() * 0.1;
        velocities[i * 3 + 2] = 0;
    }

    particles.setAttribute('position', new THREE.BufferAttribute(positions, 3));
    particles.setAttribute('velocity', new THREE.BufferAttribute(velocities, 3));

    const material = new THREE.PointsMaterial({
        color: color,
        size: size,
        transparent: true,
        opacity: 0.8
    });

    return new THREE.Points(particles, material);
};

// 创建雨、雪和雾的粒子系统
const rainParticles = createParticles(100000, 0.3, 0xff0000);
scene.add(rainParticles);

const snowParticles = createParticles(100000, 0.2, 0x0000FF);
scene.add(snowParticles);

const fogParticles = createParticles(50000, 0.2, 0x00ff00);
scene.add(fogParticles);

// 更新粒子系统
const updateParticles = (particles) => {
    const positions = particles.geometry.attributes.position.array;
    const velocities = particles.geometry.attributes.velocity.array;

    for (let i = 0; i < positions.length / 3; i++) {
        positions[i * 3 + 1] += velocities[i * 3 + 1];

        if (positions[i * 3 + 1] < -100) {
            positions[i * 3 + 1] = 100;
        }
    }

    particles.geometry.attributes.position.needsUpdate = true;
};


controls = new OrbitControls( camera, renderer.domElement );
controls.target.set( 0, 10, 0 );
controls.minDistance = 25;
controls.maxDistance = 15000;
controls.maxPolarAngle = Math.PI / 1.7;
controls.autoRotate = false;
controls.autoRotateSpeed = - 1;
controls.update();

stats = new Stats();
document.body.appendChild( stats.dom );

const animate = () => {
    controls.update();
    requestAnimationFrame(animate);

    updateParticles(rainParticles);
    updateParticles(snowParticles);
    updateParticles(fogParticles);

    renderer.render(scene, camera);

    stats.update();
};

animate();
