# 基于 YOLOv26 的钢材表面缺陷检测 Android App

## 项目文件结构说明
- `yolo26n.ipynb`，`yolo26n-tune.ipynb`，`yolo26s.ipynb`，`yolo26s-tune.ipynb`：各模型的训练代码，以及对应的调节超参数的代码
- `yolo26n`，`yolo26n-tune`，`yolo26s`，`yolo26s-tune`：训练模型时的过程文件夹，包括最新模型，最佳模型，模型权重文件，训练集评估结果，验证集评估结果
- `NCNN.ipynb`：用于模型格式转换的代码
- `result.jpg`：用于测试模型格式转换的样例图片
- `yolo26akp`：构建安卓app的项目文件夹
- `app-debug.apk`：最终输出结果，可安装在安卓手机上的安装包
- `README.md`：本说明文件

## 1，数据集

### 1.1，数据集概况

由东北大学（NEU）发布的表面缺陷数据库，收集了热轧钢带的六种典型表面缺陷，即轧制氧化皮（RS），斑块（Pa），开裂（Cr），点蚀表面（ PS），内含物（In）和划痕（Sc）。该数据库包括1,800个灰度图像：六种不同类型的典型表面缺陷，每一类缺陷包含300个样本。

<img width="930" height="609" alt="image" src="https://github.com/user-attachments/assets/87611f27-ad0b-4164-a75e-7323e7f66401" />

### 1.2，带标注的数据集

数据集已经划分为训练集和测试集，原始数据集的标签格式是 XML，通过数据集里的 xml2yolo.py 转换为 TXT

NEU-CLS（分类，只图片）：没有边界框标注，只能用于图像分类，无法直接用于 YOLO 的目标检测（缺陷定位+分类）

NEU-DET（检测，带标注）：同一批图像，但带黄色边界框 + 绿色类别标注

<img width="823" height="617" alt="image" src="https://github.com/user-attachments/assets/c6d42193-623c-4c9a-9ae9-1c0be5d7061c" />

### 1.3，数据分布情况

【训练集】

图片数量: 1770

标签文件数量: 1770

总目标框数量: 4125

空标签文件数量（无缺陷）: 0

各类别目标数量：

 0 - crazing        :    681 个目标框
 
 1 - inclusion      :    996 个目标框
 
 2 - patches        :    864 个目标框
 
 3 - pitted_surface :    424 个目标框
 
 4 - rolled-in_scale:    619 个目标框
 
 5 - scratches      :    541 个目标框

<img width="712" height="424" alt="image" src="https://github.com/user-attachments/assets/95c476a2-f68d-4418-ac21-c8af8596d665" />

【验证集】

图片数量: 30

标签文件数量: 30

总目标框数量: 64

空标签文件数量（无缺陷）: 0

各类别目标数量：

 0 - crazing        :      8 个目标框
 
 1 - inclusion      :     15 个目标框
 
 2 - patches        :     17 个目标框
 
 3 - pitted_surface :      8 个目标框
 
 4 - rolled-in_scale:      9 个目标框
 
 5 - scratches      :      7 个目标框

<img width="712" height="424" alt="image" src="https://github.com/user-attachments/assets/de1004d2-3f31-4bc5-a01a-d8b413c9dab8" />

## 2，数据预处理

标注格式转换（XML → YOLO txt）：把官方 VOC XML 转为每个图像对应的 .txt 文件

数据集目录结构整理：建立 train/images/、train/labels/、valid/images/、valid/labels/ 、data.yaml

清理重复标签：避免训练警告和潜在的 label 噪声

数据集划分（train/val）：确保验证集独立，避免过拟合。

## 3，模型训练

### 3.1，data.yaml文件

<img width="836" height="692" alt="image" src="https://github.com/user-attachments/assets/114404cd-f119-479a-97ef-7adf07337240" />

### 3.2，模型选择

YOLO26 系列所有模型架构相同，区别只在于模型规模。模型越大：参数越多、计算量越大、精度越高、速度越慢、显存占用越多

采用n和s模型进行训练，因为未来部署到边缘设备或工业相机时，可以平衡速度和精度

<img width="1932" height="678" alt="image" src="https://github.com/user-attachments/assets/55230545-ca02-4bfb-b073-efd9836e8ee2" />

### 3.3，超参数的确定

epochs=500：小数据集+使用了早停法，所以设个较大的数

imgsz=640：原图 200×200，640 是最佳平衡点，分辨率足够提取细微缺陷，又不会过度消耗显存。1280 精度略高但训练慢 2~3 倍

batch=16：显存为16g，batch 越大训练越稳定、梯度噪声越小。但过大会 OOM 或泛化变差

device=0：使用GPU

patience=50：数据小，容易在 150 epoch 左右达到峰值。50 能有效防止过拟合又不会过早停止

<img width="817" height="407" alt="image" src="https://github.com/user-attachments/assets/e2ad408b-1f06-471a-8c56-b0b6f9606647" />

### 3.4，最佳超参数（自动调参）

使用Ultralytics YOLO26 官方自动调参功能，通过遗传算法自动搜索最优超参数

<img width="982" height="711" alt="image" src="https://github.com/user-attachments/assets/79d84aa0-bb11-4b8f-bd30-dabf7a02daa8" />

<img width="1770" height="1080" alt="image" src="https://github.com/user-attachments/assets/dda29564-b371-41de-8468-c13c1afde0c2" />

### 3.5，对照试验

<img width="1752" height="429" alt="image" src="https://github.com/user-attachments/assets/d0e619cb-30ce-42be-a066-b5157835f9ad" />

## 4，模型评估

### 4.1，yolo26n（默认超参数）

<img width="1176" height="513" alt="image" src="https://github.com/user-attachments/assets/6c219d16-53fc-4629-aebe-ee9abc8d696c" />

<img width="1170" height="312" alt="image" src="https://github.com/user-attachments/assets/fc2129af-f409-4eef-9367-01c673ceff52" />

<img width="1428" height="498" alt="image" src="https://github.com/user-attachments/assets/eb3ecb59-dbb0-4a65-9884-25a7bb10fa25" />

### 4.2，yolo26n（最佳超参数）

<img width="1176" height="588" alt="image" src="https://github.com/user-attachments/assets/6dcc8054-1446-4ca4-8156-4a6c705bc949" />

<img width="1188" height="507" alt="image" src="https://github.com/user-attachments/assets/ff83ed4f-1bad-4faf-9a3d-b7457be07256" />

<img width="1181" height="321" alt="image" src="https://github.com/user-attachments/assets/fe1fbc5d-fc01-460f-a284-205614df4577" />

<img width="1359" height="503" alt="image" src="https://github.com/user-attachments/assets/89a19508-742d-4a2b-9a66-6a8779b37a04" />

### 4.3，yolo26s（默认超参数）

<img width="1179" height="510" alt="image" src="https://github.com/user-attachments/assets/523adbef-d2fc-4eb7-9320-5fc373748249" />

<img width="1176" height="324" alt="image" src="https://github.com/user-attachments/assets/91364e34-65a9-4403-986d-78aa1cf8f49a" />

<img width="1356" height="510" alt="image" src="https://github.com/user-attachments/assets/be466768-6507-4e8c-a30b-1eb341f7fbf6" />

### 4.4，yolo26s（最佳超参数）

<img width="1179" height="576" alt="image" src="https://github.com/user-attachments/assets/c5d4d9c8-eb93-4908-8870-0d5135a98466" />

<img width="1179" height="507" alt="image" src="https://github.com/user-attachments/assets/55c2d046-fdc0-4a31-b30d-6d00e21da16d" />

<img width="1170" height="336" alt="image" src="https://github.com/user-attachments/assets/d76546aa-08d4-4dfe-a810-8a6a8e75d513" />

<img width="945" height="351" alt="image" src="https://github.com/user-attachments/assets/6cec1e87-6888-4e9d-acf0-cc438746d903" />

### 4.5，总体评估

<img width="1614" height="576" alt="image" src="https://github.com/user-attachments/assets/bcb02fc6-afc6-48eb-90c6-1787faaedc96" />

结论：

最佳模型是YOLO26s-tune，综合性能最优，Recall最高（漏检最少），工业上漏检代价远高于误检

YOLO26n-tune表现次之，说明对超参数进行调优对n模型也有帮助，但受限于模型规模，仍然逊于s模型

<img width="1632" height="906" alt="image" src="https://github.com/user-attachments/assets/13af0581-aa86-4bd5-9d3e-43ac8a9c92a1" />

结论：

inclusion是最难的一类，调参后在 s 模型上提升最大

crazing和 scratches是 n 模型的强项，s 调参版反而变弱

patches / pitted_surface 在所有模型中都接近完美（>0.94），说明这些缺陷特征非常明显

rolled-in_scale 在 s + 调参版提升最明显

# 5，安卓app的开发

## 5.1，模型转换

将模型转换为 NCNN 格式（model.ncnn.param/bin），用于移动端部署。

<img width="1185" height="447" alt="image" src="https://github.com/user-attachments/assets/d89be13c-ca8c-48f2-802a-180e15393f6c" />

<img width="1143" height="354" alt="image" src="https://github.com/user-attachments/assets/d4f5962b-cda7-4cec-ac32-d91747b14700" />

## 5.2，app开发

参考ncnn-android-yolov8 开源模板

修改核心 C++ 代码（yolov8ncnn.cpp、yolov8_det.cpp），实现自定义模型固定加载、640×640 输入尺寸适配及 6 类缺陷后处理与绘制逻辑。将检测框坐标映射回原图，输出类别与置信度结果。

<img width="2559" height="1526" alt="image" src="https://github.com/user-attachments/assets/dfa2d8e3-5100-4c06-b49a-32ee9155eeef" />

## 5.3，app测试

完成 Android 端打包、安装测试与 Redmi K60 真机适配，实现“拍照 -> 推理 -> 绘制结果 -> 本地相册保存”完整流程。

![Screenshot_2026-04-14-00-18-35-268_com example steeldefect](https://github.com/user-attachments/assets/5980a5ed-6859-4374-bdea-b644e70b264a)

![steel_defect_1776096945478](https://github.com/user-attachments/assets/45079c0f-89c8-4497-a85a-dacbc1aa2ec3)

![steel_defect_1776096690024_1776096720208edit](https://github.com/user-attachments/assets/8525165d-b618-457f-b56b-9a2bfad3df2a)

![steel_defect_1776096514331_1776096540102edit](https://github.com/user-attachments/assets/63e78152-d1cb-42f5-a1d9-f6a88cd9ed1d)
