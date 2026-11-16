# 3D 原型资产与限制

- 运行库：Three.js 0.180.0（MIT）、@pixiv/three-vrm 3.4.4（MIT）。完整许可随依赖保留。
- 测试模型：VRM1_Constraint_Twist_Sample v1.0.1，© 2022 pixiv Inc.。
- 来源：https://github.com/pixiv/three-vrm/blob/dev/packages/three-vrm/examples/models/VRM1_Constraint_Twist_Sample.vrm
- 模型遵守其内嵌 VRM 1.0 许可与元数据，不等同于运行库 MIT 许可：https://vrm.dev/licenses/1.0/ 。内嵌元数据允许所有人使用、企业商业使用、修改及再分发，禁止反社会或仇恨使用。
- 当前为技术测试角色，并非用户选定静态图片 A 的绑定版本，也不是最终银蓝发角色。
- 音量/频谱推导的口型是近似元音混合，不是音素识别，不宣称逐字精准唇读同步。
- “无声动作预览”只检验模型驱动，不播放声音，也不代表真实对话验收。
- 页面帧率必须在目标板实际读取；开发机结果不能用作 RK3576 性能结论。
- 状态服务仅监听板端回环地址；所有运行资源可本地加载，无需上传语音到新增服务。