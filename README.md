# 使用YOLO26的钢材表面缺陷检测

## 1，数据集

### 1.1，数据集概况

由东北大学（NEU）发布的表面缺陷数据库，收集了热轧钢带的六种典型表面缺陷，即轧制氧化皮（RS），斑块（Pa），开裂（Cr），点蚀表面（ PS），内含物（In）和划痕（Sc）。该数据库包括1,800个灰度图像：六种不同类型的典型表面缺陷，每一类缺陷包含300个样本。

<img width="930" height="609" alt="image" src="https://github.com/user-attachments/assets/8a8bb677-e2ac-4601-b1b2-3f0a1b1b191a" />

### 1.2，带标注的数据集

数据集已经划分为训练集和测试集，原始数据集的标签格式是 XML，通过数据集里的 xml2yolo.py 转换为 TXT

NEU-CLS（分类，只图片）：没有边界框标注，只能用于图像分类，无法直接用于 YOLO 的目标检测（缺陷定位+分类）

NEU-DET（检测，带标注）：同一批图像，但带黄色边界框 + 绿色类别标注

<img width="823" height="617" alt="image" src="https://github.com/user-attachments/assets/19c9491d-62cb-4ca6-9d7f-2c24420f6dae" />

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

<img width="712" height="424" alt="image" src="https://github.com/user-attachments/assets/99f762dd-cace-4203-8c15-fb9ca301eaab" />

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

<img width="712" height="424" alt="image" src="https://github.com/user-attachments/assets/a51ae910-86d5-44c1-8e6e-6a71fefe2b40" />

## 2，数据预处理

标注格式转换（XML → YOLO txt）：把官方 VOC XML 转为每个图像对应的 .txt 文件

数据集目录结构整理：建立 train/images/、train/labels/、valid/images/、valid/labels/ 、data.yaml

清理重复标签：避免训练警告和潜在的 label 噪声

数据集划分（train/val）：确保验证集独立，避免过拟合。

## 3，模型训练

### 3.1，data.yaml文件

<img width="836" height="692" alt="image" src="https://github.com/user-attachments/assets/bf4bebd3-af93-4725-8f40-9f3e891f3ae9" />

### 3.2，模型选择

YOLO26 系列所有模型架构相同，区别只在于模型规模。模型越大：参数越多、计算量越大、精度越高、速度越慢、显存占用越多

采用n和s模型进行训练，因为未来部署到边缘设备或工业相机时，可以平衡速度和精度

<img width="1928" height="678" alt="image" src="https://github.com/user-attachments/assets/8fdde164-d9ba-4553-8780-1a3f1dd4b43d" />

### 3.3，超参数的确定

epochs=500：小数据集+使用了早停法，所以设个较大的数

imgsz=640：原图 200×200，640 是最佳平衡点，分辨率足够提取细微缺陷，又不会过度消耗显存。1280 精度略高但训练慢 2~3 倍

batch=16：显存为16g，batch 越大训练越稳定、梯度噪声越小。但过大会 OOM 或泛化变差

device=0：使用GPU

patience=50：数据小，容易在 150 epoch 左右达到峰值。50 能有效防止过拟合又不会过早停止

<img width="817" height="407" alt="image" src="https://github.com/user-attachments/assets/87dfa213-a152-47c2-b554-6d863a97946b" />

### 3.4，最佳超参数（自动调参）

使用Ultralytics YOLO26 官方自动调参功能，通过遗传算法自动搜索最优超参数

<img width="982" height="711" alt="image" src="https://github.com/user-attachments/assets/9c4e1f02-2fd8-4e96-b568-d8a8f0be2fc2" />

<img width="1770" height="1080" alt="image" src="https://github.com/user-attachments/assets/ad42a714-a8f3-42f2-a335-c7740a2cfda2" />

### 3.5，对照试验

<img width="1752" height="435" alt="image" src="https://github.com/user-attachments/assets/06b43993-c9b0-477d-874d-f44361466f79" />

## 4，模型评估

### 4.1，yolo26n（默认超参数）

<img width="1182" height="506" alt="image" src="https://github.com/user-attachments/assets/2b07d893-4ff7-42e9-82af-4260e5034792" />

<img width="1170" height="324" alt="image" src="https://github.com/user-attachments/assets/5cdc77bc-dd2d-4f05-888e-2238fc9e35c5" />

<img width="1143" height="789" alt="image" src="https://github.com/user-attachments/assets/de954dd2-3c01-4df3-a534-55ca5adbfc0a" />


### 4.2，yolo26n（最佳超参数）

<img width="1179" height="582" alt="image" src="https://github.com/user-attachments/assets/7885d422-d861-4e0a-a56e-d508aeb4246f" />

<img width="1175" height="516" alt="image" src="https://github.com/user-attachments/assets/cf456eaf-ed86-47dd-a825-359f62680f99" />

<img width="1173" height="321" alt="image" src="https://github.com/user-attachments/assets/7a0fb046-e368-4814-8992-e06505dd58f3" />

<img width="1140" height="765" alt="image" src="https://github.com/user-attachments/assets/b139913c-e693-4e12-a9f3-0aec76e023e2" />

### 4.3，yolo26s（默认超参数）

<img width="1176" height="509" alt="image" src="https://github.com/user-attachments/assets/89465aea-6378-45a6-8e17-4a41079a6095" />

<img width="1176" height="324" alt="image" src="https://github.com/user-attachments/assets/3bbdee15-e943-44df-9189-6f5c8d0e6a39" />

<img width="1143" height="767" alt="image" src="https://github.com/user-attachments/assets/dd40be93-275e-486b-a08a-189104e1ad4c" />

### 4.4，yolo26s（最佳超参数）

<img width="1185" height="579" alt="image" src="https://github.com/user-attachments/assets/d08f59a1-0b50-4470-92de-d69cdd55fb4b" />

<img width="1179" height="513" alt="image" src="https://github.com/user-attachments/assets/0809d544-68d9-48a8-b1d3-11215047e042" />

<img width="1179" height="338" alt="image" src="https://github.com/user-attachments/assets/7346f484-2c99-4750-b24f-9216405f2945" />

<img width="1137" height="762" alt="image" src="https://github.com/user-attachments/assets/a0b261c0-bce0-4285-87e8-1adb6e02c0a2" />

### 4.5，总体评估

<img width="1625" height="576" alt="image" src="https://github.com/user-attachments/assets/103cccb1-af05-4362-8ca2-4adbbe18d526" />

结论：

最佳模型是YOLO26s-tune，综合性能最优，Recall最高（漏检最少），工业上漏检代价远高于误检

YOLO26n-tune表现次之，说明对超参数进行调优对n模型也有帮助，但受限于模型规模，仍然逊于s模型


<img width="1647" height="891" alt="image" src="https://github.com/user-attachments/assets/5af441bd-b366-4a6c-9ce6-86b79074659b" />

结论：

inclusion是最难的一类，调参后在 s 模型上提升最大

crazing和 scratches是 n 模型的强项，s 调参版反而变弱

patches / pitted_surface 在所有模型中都接近完美（>0.94），说明这些缺陷特征非常明显

rolled-in_scale 在 s + 调参版提升最明显

