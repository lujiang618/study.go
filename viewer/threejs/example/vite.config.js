// vite.config.js
import { defineConfig } from "vite";
import topLevelAwait from 'vite-plugin-top-level-await';

export default defineConfig({
    server: {
        port: 5173, // 自定义端口，默认为5173
        open: true, // 服务启动后，自动在浏览器中打开，默认是不打开的
        hmr: true, // 为开发服务启用热更新，默认是不启用热更新的
    },
    build: {
        target: ["chrome89", "edge89", "firefox89", "safari15"],
    },
    plugins: [topLevelAwait()],
});
