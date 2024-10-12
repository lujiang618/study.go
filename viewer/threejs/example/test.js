import * as THREE from "three";


// 创建场景、相机和渲染器
const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
const renderer = new THREE.WebGLRenderer();
renderer.setSize(window.innerWidth, window.innerHeight);
document.body.appendChild(renderer.domElement);

// 粒子系统基类
class ParticleSystem {
    constructor(count, geometry, material, emitter) {
        this.count = count;
        this.geometry = geometry; // 粒子的几何体
        this.material = material; // 粒子的材质
        this.emitter = emitter;   // 粒子发射器
        this.instancedMesh = new THREE.InstancedMesh(this.geometry, this.material, this.count);

        // 用于存储每个实例的变换矩阵
        this.dummy = new THREE.Object3D();

        // 初始化粒子
        this.initParticles();
        scene.add(this.instancedMesh);
    }

    initParticles() {
        for (let i = 0; i < this.count; i++) {
            this.resetParticle(i);
        }
        this.instancedMesh.instanceMatrix.needsUpdate = true;
    }

    resetParticle(index) {
        const position = this.emitter.getPosition();
        this.dummy.position.copy(position);
        this.dummy.scale.setScalar(Math.random() * 0.5 + 0.5); // 随机缩放

        // 随机旋转
        this.dummy.rotation.x = Math.random() * Math.PI;
        this.dummy.rotation.y = Math.random() * Math.PI;
        this.dummy.rotation.z = Math.random() * Math.PI;

        this.dummy.updateMatrix();
        this.instancedMesh.setMatrixAt(index, this.dummy.matrix);
    }

    update() {
        for (let i = 0; i < this.count; i++) {
            this.updateParticle(i);
        }
        this.instancedMesh.instanceMatrix.needsUpdate = true; // 通知 Three.js 更新实例矩阵
    }

    updateParticle(index) {
        this.instancedMesh.getMatrixAt(index, this.dummy.matrix);
        this.dummy.position.add(this.dummy.userData.velocity);
        this.dummy.updateMatrix();
        this.instancedMesh.setMatrixAt(index, this.dummy.matrix);
    }
}

// 粒子发射器基类
class ParticleEmitter {
    constructor() {}

    getPosition() {
        return new THREE.Vector3(0, 0, 0); // 默认位置
    }
}

// 球形发射器
class SphereEmitter extends ParticleEmitter {
    constructor(radius) {
        super();
        this.radius = radius;
    }

    getPosition() {
        const theta = Math.random() * Math.PI * 2;
        const phi = Math.acos(Math.random() * 2 - 1);
        const x = this.radius * Math.sin(phi) * Math.cos(theta);
        const y = this.radius * Math.cos(phi);
        const z = this.radius * Math.sin(phi) * Math.sin(theta);
        return new THREE.Vector3(x, y, z);
    }
}

// 创建粒子几何体和材质的工厂函数
function createGeometry() {
    return new THREE.CircleGeometry(0.5, 6); // 粒子几何
}

function createMaterial() {
    return new THREE.MeshBasicMaterial({ color: 0xffffff, transparent: true, opacity: 0.8 }); // 粒子材质
}

// 创建粒子特效实例
const sphereEmitter = new SphereEmitter(50);
const geometry = createGeometry();
const material = createMaterial();
const snowSystem = new ParticleSystem(10000, geometry, material, sphereEmitter); // 10000 个粒子

// 相机位置
camera.position.z = 150;

// 动画循环
function animate() {
    requestAnimationFrame(animate);
    snowSystem.update();
    renderer.render(scene, camera);
}
animate();
