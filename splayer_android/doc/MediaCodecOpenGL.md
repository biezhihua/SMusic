

OpenGL渲染MediaCodec解码数据
    1. OpenGL生成纹理
    2. 纹理绑定到SurfaceTexture
    3. 用SurfaceTexture做参数创建Surface
    4. MediaCodec解码的视频往Surface发送，显示画面