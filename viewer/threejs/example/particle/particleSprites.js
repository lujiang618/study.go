import * as THREE from "three";

import Stats from "three/addons/libs/stats.module.js";

import { GUI } from "three/addons/libs/lil-gui.module.min.js";

let camera, scene, renderer, stats, parameters;
let mouseX = 0,
    mouseY = 0;

let windowHalfX = window.innerWidth / 2;
let windowHalfY = window.innerHeight / 2;

const materials = [];

init();

function init() {
    camera = new THREE.PerspectiveCamera(
        75,
        window.innerWidth / window.innerHeight,
        1,
        20000
    );
    camera.position.z = 1000;

    scene = new THREE.Scene();
    scene.fog = new THREE.FogExp2(0x000000, 0.0008);

    const geometry = new THREE.BufferGeometry();
    const vertices = [];

    const textureLoader = new THREE.TextureLoader();

    const assignSRGB = (texture) => {
        texture.colorSpace = THREE.SRGBColorSpace;
    };

    const sprite1 = textureLoader.load(
        "../textures/sprites/snowflake1.png",
        assignSRGB
    );
    const sprite2 = textureLoader.load(
        "../textures/sprites/snowflake2.png",
        assignSRGB
    );
    const sprite3 = textureLoader.load(
        "../textures/sprites/snowflake3.png",
        assignSRGB
    );
    const sprite4 = textureLoader.load(
        "../textures/sprites/snowflake4.png",
        assignSRGB
    );
    const sprite5 = textureLoader.load(
        "t../extures/sprites/snowflake5.png",
        assignSRGB
    );

    for (let i = 0; i < 10000; i++) {
        const x = Math.random() * 2000 - 1000;
        const y = Math.random() * 2000 - 1000;
        const z = Math.random() * 2000 - 1000;

        vertices.push(x, y, z);
    }

    geometry.setAttribute(
        "position",
        new THREE.Float32BufferAttribute(vertices, 3)
    );

    parameters = [
        [[1.0, 0.2, 0.5], sprite2, 20],
        [[0.95, 0.1, 0.5], sprite3, 15],
        [[0.9, 0.05, 0.5], sprite1, 10],
        [[0.85, 0, 0.5], sprite5, 8],
        [[0.8, 0, 0.5], sprite4, 5],
    ];

    for (let i = 0; i < parameters.length; i++) {
        const color = parameters[i][0];
        const sprite = parameters[i][1];
        const size = parameters[i][2];

        materials[i] = new THREE.PointsMaterial({
            size: size,
            map: sprite,
            blending: THREE.AdditiveBlending,
            depthTest: false,
            transparent: true,
        });
        materials[i].color.setHSL(
            color[0],
            color[1],
            color[2],
            THREE.SRGBColorSpace
        );

        const particles = new THREE.Points(geometry, materials[i]);

        // 为每个粒子赋予一个随机的旋转效果。
        particles.rotation.x = Math.random() * 6;
        particles.rotation.y = Math.random() * 6;
        particles.rotation.z = Math.random() * 6;

        scene.add(particles);
    }

    //

    renderer = new THREE.WebGLRenderer();
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(window.innerWidth, window.innerHeight);
    renderer.setAnimationLoop(animate);
    document.body.appendChild(renderer.domElement);

    //

    stats = new Stats();
    document.body.appendChild(stats.dom);

    //

    const gui = new GUI();

    const params = {
        texture: true,
    };

    gui.add(params, "texture").onChange(function (value) {
        for (let i = 0; i < materials.length; i++) {
            materials[i].map = value === true ? parameters[i][1] : null;
            materials[i].needsUpdate = true;
        }
    });

    gui.open();

    document.body.style.touchAction = "none";
    document.body.addEventListener("pointermove", onPointerMove);

    //

    window.addEventListener("resize", onWindowResize);
}

function onWindowResize() {
    windowHalfX = window.innerWidth / 2;
    windowHalfY = window.innerHeight / 2;

    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();

    renderer.setSize(window.innerWidth, window.innerHeight);
}

function onPointerMove(event) {
    if (event.isPrimary === false) return;

    mouseX = event.clientX - windowHalfX;
    mouseY = event.clientY - windowHalfY;
}

//

function animate() {
    render();
    stats.update();
}

function calculateAngleWithXYPlane(x, y, z) {
    // 计算点向量的长度
    const vectorLength = Math.sqrt(x * x + y * y + z * z);
    // 计算余弦值
    const cosTheta = y / vectorLength;
    // 计算夹角（弧度）
    const thetaRadians = Math.acos(cosTheta);
    // 将弧度转换为角度
    const thetaDegrees = thetaRadians * (180 / Math.PI);

     // 判断方向
     let angle = thetaDegrees;
     if (y < 0) {
         angle = 360 - thetaDegrees;
     }

    return angle;
}

function render() {
    camera.position.x += (mouseX - camera.position.x) * 0.05;
    camera.position.y += (-mouseY - camera.position.y) * 0.05;
    camera.lookAt(scene.position);

    const time = Date.now() * 0.00005;
    for (let i = 0; i < scene.children.length; i++) {
        const object = scene.children[i];

        if (object instanceof THREE.Points) {
            // object.rotation.y = time * (i < 4 ? i + 1 : -(i + 1));
            // object.rotation.y = time * -5;
            // object.rotation.y = time * -1;
            object.rotation.x += 0.002;
        }

        if (i===0) {
            // const angle = calculateAngleWithXYPlane(object.rotation.x, object.rotation.y, object.rotation.z)
            // console.log(i, object.rotation.y, object.rotation.z, object.rotation.x)
        }

    }

    for (let i = 0; i < materials.length; i++) {
        const color = parameters[i][0];

        const h = ((360 * (color[0] + time)) % 360) / 360;
        materials[i].color.setHSL(h, color[1], color[2], THREE.SRGBColorSpace);
    }

    renderer.render(scene, camera);
}
