import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import Stats from 'three/addons/libs/stats.module.js';


let scene, camera, renderer, rockets = [], fireworks = [];

scene = new THREE.Scene();
scene.background = new THREE.Color(0x002244); // 深蓝色背景

camera = new THREE.PerspectiveCamera(60, window.innerWidth / window.innerHeight, 44, 200);
camera.position.z = 10;

renderer = new THREE.WebGLRenderer({ antialias: true });
renderer.setSize(window.innerWidth, window.innerHeight);
document.body.appendChild(renderer.domElement);

const ambientLight = new THREE.AmbientLight(0xffffff, 0.8);
scene.add(ambientLight);

const pointLight = new THREE.PointLight(0xffffff, 1.5, 100);
pointLight.position.set(10, 10, 10);
scene.add(pointLight);



function createFirework() {
    const particleCount = 200;
    const particles = new THREE.BufferGeometry();
    const positions = new Float32Array(particleCount * 3);
    const velocities = new Float32Array(particleCount * 3);
    const colors = new Float32Array(particleCount * 3);

    for (let i = 0; i < particleCount; i++) {
        const theta = Math.random() * 2 * Math.PI;
        const phi = Math.acos((Math.random() * 2) - 1);
        const distance = Math.random() * 20;

        positions[i * 3] = distance * Math.sin(phi) * Math.cos(theta);
        positions[i * 3 + 1] = distance * Math.sin(phi) * Math.sin(theta);
        positions[i * 3 + 2] = distance * Math.cos(phi);

        velocities[i * 3] = positions[i * 3] * 0.1;
        velocities[i * 3 + 1] = positions[i * 3 + 1] * 0.1;
        velocities[i * 3 + 2] = positions[i * 3 + 2] * 0.1;

        colors[i * 3] = Math.random();
        colors[i * 3 + 1] = Math.random();
        colors[i * 3 + 2] = Math.random();
    }

    particles.setAttribute('position', new THREE.BufferAttribute(positions, 3));
    particles.setAttribute('velocity', new THREE.BufferAttribute(velocities, 3));
    particles.setAttribute('color', new THREE.BufferAttribute(colors, 3));

    const material = new THREE.PointsMaterial({
        size: 1.0,
        vertexColors: true,
        transparent: true,
        opacity: 1
    });

    const particleSystem = new THREE.Points(particles, material);
    scene.add(particleSystem);
    console.log(particleSystem)

    fireworks.push(particleSystem);
}

createFirework();

function animate() {
    requestAnimationFrame(animate);



    fireworks.forEach(firework => {
        const positions = firework.geometry.attributes.position.array;
        const velocities = firework.geometry.attributes.velocity.array;

        for (let i = 0; i < positions.length; i += 3) {
            positions[i] += velocities[i];
            positions[i + 1] += velocities[i + 1];
            positions[i + 2] += velocities[i + 2];

            velocities[i + 1] -= 0.005;
            velocities[i] *= 0.98;
            velocities[i + 1] *= 0.98;
            velocities[i + 2] *= 0.98;
        }

        firework.geometry.attributes.position.needsUpdate = true;
        firework.material.opacity -= 0.01;
        if (firework.material.opacity <= 0) {
            scene.remove(firework);
            fireworks = fireworks.filter(fw => fw !== firework);
        }
    });

    renderer.render(scene, camera);
}

animate()

window.addEventListener('resize', onWindowResize);

function onWindowResize() {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
}

