rtspServer
这个一个rtsp服务器，能够实现多客户端链接，且随意向服务器中添加推流资源供客户端访问
编译过程
源码中提供了两种CMakeLists.txt
CMakeLists_local.txt : 在本地编译运行
CMakeLists_arm.txt : 交叉编译运行，修改编译工具链为适合自己平台的工具链即可
执行如下命令编译：
cmake ../ -DCMAKE_INSTALL_PREFIX=../arm_install -DSYNC_MODE=AV_SYNC_AUDIO_MASTER
SYNC_MODE宏为指定音视频同步的方案：音频为基准(AV_SYNC_AUDIO_MASTER)，视频为基准(AV_SYNC_VIDEO_MASTER)，外部时钟为基准(AV_SYNC_EXTERNAL_MASTER)
rtsp服务器URL：rtsp://localip:8554/sourcename; 例如添加了一个视频源，资源名命名为test_media;则这个资源的访问路径为：rtsp://localip:8554/test_media
