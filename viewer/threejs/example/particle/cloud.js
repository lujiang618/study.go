import * as THREE from "three";
import { OrbitControls } from "three/addons/controls/OrbitControls.js";
import Stats from "three/addons/libs/stats.module.js";
import { mergeGeometries } from 'three/examples/jsm/utils/BufferGeometryUtils.js';

// 背景色，目前为天蓝色
const BackGroundColor = "#1e4877";
const CloudCount = 10;
// X轴和Y轴平移的随机参数
const RandomPositionX = 80;
const RandomPositionY = 120;
// 每个云所占z轴的长度
const perCloudZ = 15;
// 所有的云一共的Z轴长度
const cameraPositionZ = CloudCount * perCloudZ;
const StartTime = Date.now();

// 透视相机，只有距离相机1~500的物体才可以被渲染
const camera = new THREE.PerspectiveCamera(70, window.innerWidth / window.innerHeight, 1, 1000);
// 相机的位置，平移下左右平衡
camera.position.x = Math.floor(RandomPositionX / 2);
// 最初在最远处
camera.position.z = cameraPositionZ;

// 把线性雾改成指数雾
// THREE.FogExp2 的参数是：颜色和密度
// 密度值通常很小，0.001 到 0.1 之间
const fog = new THREE.FogExp2(BackGroundColor, 0.002);

const scene = new THREE.Scene();
scene.background = new THREE.Color(BackGroundColor);
scene.fog = fog;  // 记得把雾添加到场景中

const cloud = "../textures/cloud.png";
const texture = new THREE.TextureLoader().load(cloud);

// 一个平面形状
const geometry = new THREE.PlaneGeometry(64, 64);
const geometries = [];

const vShader = `
  varying vec2 vUv;
  void main()
  {
    vUv = uv;
    gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
  }
`;
const fShader = `
  uniform sampler2D map;
  uniform vec3 fogColor;
  uniform float fogDensity;
  varying vec2 vUv;
  void main()
  {
    float depth = gl_FragCoord.z / gl_FragCoord.w;
    float fogFactor = 1.0 - exp( - fogDensity * fogDensity * depth * depth );
    gl_FragColor = texture2D(map, vUv );
    gl_FragColor.w *= pow( gl_FragCoord.z, 20.0 );
    gl_FragColor = mix( gl_FragColor, vec4( fogColor, gl_FragColor.w ), fogFactor );
  }
`;
// 贴图材质
const material = new THREE.ShaderMaterial({
    // 这里的值是给着色器传递的
    uniforms: {
        map: {
            type: "t",
            value: texture,
        },
        fogColor: {
            type: "c",
            value: fog.color,
        },
        fogDensity: {
            type: "f",
            value: fog.density,
        },
    },
    vertexShader: vShader,
    fragmentShader: fShader,
    transparent: true,
    depthWrite: false,
    side: THREE.DoubleSide
});



for (var i = 0; i < CloudCount; i++) {
    const instanceGeometry = geometry.clone();

    // 把这个克隆出来的云，通过随机参数，做一些位移，达到一堆云彩的效果，每次渲染出来的云堆都不一样
    // X轴偏移后，通过调整相机位置达到平衡
    // Y轴想把云彩放在场景的偏下位置，所以都是负值
    // Z轴位移就是：当前第几个云*每个云所占的Z轴长度
    instanceGeometry.translate(
        (Math.random() - 0.5 ) * i * RandomPositionX,
        (Math.random() - 0.5 ) * i * RandomPositionY,
        i * perCloudZ
    );

    geometries.push(instanceGeometry);
}

// 把这些形状合并
const mergedGeometry = mergeGeometries(geometries);

// 把上面合并出来的形状和材质，生成一个物体
const mesh = new THREE.Mesh(mergedGeometry, material);
// 添加进场景
scene.add(mesh);

const renderer = new THREE.WebGLRenderer({ antialias: false });
renderer.setSize(window.innerWidth, window.innerHeight);
document.body.appendChild(renderer.domElement);

let controls, stats;
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

function animate() {
    requestAnimationFrame(animate);

    controls.update();
    stats.update();
    // 从最远的z轴处开始往前一点一点的移动，达到穿越云层的目的
    // camera.position.z =
        // cameraPositionZ - (((Date.now() - StartTime) * 0.03) % cameraPositionZ);

    // 添加以下代码使云朝向相机
    mesh.quaternion.copy(camera.quaternion);

    renderer.render(scene, camera);
}

animate();