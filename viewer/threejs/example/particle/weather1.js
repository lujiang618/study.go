import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
// import Stats from 'three/addons/libs/stats.module.js';
import Stats from 'stats-gl';

// 创建场景
const scene = new THREE.Scene();

// 创建相机
const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 100000);
camera.position.x = 336;
camera.position.y = 748;
camera.position.z = 1050;

// 创建渲染器
const renderer = new THREE.WebGLRenderer();
renderer.setSize(window.innerWidth, window.innerHeight);
document.body.appendChild(renderer.domElement);

// 添加环境光
const ambientLight = new THREE.AmbientLight(0x888888);
scene.add(ambientLight);

// 添加点光源
const pointLight = new THREE.PointLight(0xFFFFFF, 1, 100000);
pointLight.position.set(100, 100, 100);
scene.add(pointLight);

// 创建粒子几何体和材质
const createParticles = (count, size, color) => {
    const particles = new THREE.BufferGeometry();
    const positions = new Float32Array(count * 3);
    const velocities = new Float32Array(count * 3);

    for (let i = 0; i < count; i++) {
        positions[i * 3] = (Math.random() - 0.5) * 800;
        positions[i * 3 + 1] = (Math.random() - 0.5) * 800;
        positions[i * 3 + 2] = (Math.random() - 0.5) * 800;

        velocities[i * 3] = 0;
        velocities[i * 3 + 1] = -Math.random() * 0.1;
        velocities[i * 3 + 2] = 0;
    }

    particles.setAttribute('position', new THREE.BufferAttribute(positions, 3));
    particles.setAttribute('velocity', new THREE.BufferAttribute(velocities, 3));

    const material = new THREE.ShaderMaterial({
        uniforms: {
            color: { value: new THREE.Color(color) },
            pointSize: { value: size },
            time: { value:  0.0 }
        },
        vertexShader: `
            uniform float time;
            uniform float pointSize;
            attribute vec3 velocity;
            varying vec3 vColor;
            void main() {
                vec3 newPosition = position;
                float elapsedTime = time;

                // 如果粒子下降到 -100 以下，则将其位置重置为 100 并继续下降
                while (newPosition.y + velocity.y * elapsedTime < -400.0) {
                    elapsedTime -= (-400.0 - newPosition.y) / velocity.y;
                    newPosition.y = 400.0;
                }
                newPosition += velocity * elapsedTime;

                vColor = vec3(1.0, 1.0, 1.0);
                gl_Position = projectionMatrix * modelViewMatrix * vec4(newPosition, 1.0);
                gl_PointSize = pointSize;
            }
        `,
        fragmentShader: `
            uniform vec3 color;
            varying vec3 vColor;
            void main() {
                gl_FragColor = vec4(color * vColor, 1.0);
            }
        `,
        transparent: true
    });

    return new THREE.Points(particles, material);
};

// 创建雨、雪和雾的粒子系统
const rainParticles = createParticles(10000, 3, 0xAA0000);
scene.add(rainParticles);

const snowParticles = createParticles(10000, 0.2, 0x00ff00);
scene.add(snowParticles);

const fogParticles = createParticles(500, 1, 0x0000AA);
scene.add(fogParticles);

let controls, stats;

controls = new OrbitControls( camera, renderer.domElement );
controls.target.set( 0, 10, 0 );
controls.minDistance = 5;
controls.maxDistance = 15000;
controls.maxPolarAngle = Math.PI / 1.7;
controls.autoRotate = false;
controls.autoRotateSpeed = - 1;
controls.update();

// stats = new Stats();
// document.body.appendChild( stats.dom );


stats = new Stats( {
    precision: 3,
    horizontal: false
} );
stats.init( renderer );
document.body.appendChild( stats.dom );

// 更新粒子系统
const animate = () => {
    controls.update();
    stats.update();

    // console.log(camera.position)
    requestAnimationFrame(animate);

    const time = performance.now() * 0.1;

    rainParticles.material.uniforms.time.value = time;
    snowParticles.material.uniforms.time.value = time;
    fogParticles.material.uniforms.time.value = time;

    renderer.render(scene, camera);
};

animate();
