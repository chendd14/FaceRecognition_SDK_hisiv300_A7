# FaceRecognition SDK A7 edition, cross compiled by Ubuntu16.04LTS 32bit

### SDK授权

SDK采用软授权，执行`example`，可以获取机器码。调用SetLicenseFile函数，可以设置授权文件的名称。

### 人脸比对一般流程
```flow
st=>start: 开始
op1=>operation: cv::imread，读取图片1和图片2（in BGR format）
op2=>operation: VisionFaceDetect，获取图片1和图片2中的人脸位置
op3=>operation: VisionFaceAlignment，归一化图片1和图片2中的人脸
op4=>operation: VisionFaceFeature，获取图片1和图片2中的人脸特征
op5=>operation: VisionFaceFeature，比对两人脸特征，获取相似度
e=>end: 结束
st->op1->op2->op3->op4->op5->e
```

### 1.&nbsp;&nbsp;人脸检测
```c++
namespace vision {
    class VISION_API VisionFaceDetect
	{
	public:
		/* 默认析构函数 */
		virtual ~VisionFaceDetect() {}

		/* 初始化，输入参数文件param_path和所要使用的GPU编号device_id */
		virtual bool Init(std::string param_path = "models/face_detect_cascade.json", int device_id = 0) = 0;

		/* 输入一张图像img，获取人脸信息（包括人脸框和关键点）；在输入视频的情况下，将is_video_stream设置为true，能够极大提升检测速度 */
		virtual std::vector<VisionFace> GetFaces(const cv::Mat& img, bool is_video_stream = false) = 0;

		/* 输入一张图像img，返回所有的人脸框以及对应的人脸关键点（引用形式返回） */
		virtual std::vector<cv::Rect> GetFaces(const cv::Mat& img, std::vector<std::vector<cv::Point2f>>& key_pts, bool is_video_stream = false) = 0;

		/* 获取图像img中最大人脸的人脸框 */
		virtual std::vector<VisionFace> GetMaximumFace(const cv::Mat& img, bool is_video_stream = false) = 0;

		/* 输入一张图像img，图像img中最大人脸的人脸框以及对应的人脸关键点（引用形式返回） */
		virtual std::vector<cv::Rect> GetMaximumFace(const cv::Mat& img, std::vector<std::vector<cv::Point2f>>& key_pts, bool is_video_stream = false) = 0;
	};

	/* 构造对象，默认使用GPU 0 */
	VISION_API VisionFaceDetect* instantiateVisionFaceDetect(int device_id = 0);

	/* 销毁对象 */
	VISION_API void destroyVisionFaceDetect(VisionFaceDetect* ptr);
}
```
示例代码
```c++
// 摄像头测试
void CameraTest()
{
    VisionFaceDetect *face_detect = instantiateVisionFaceDetect();
    cv::VideoCapture capture(0);
    Mat img;

    while (true)
    {
        capture >> img;

        if (!img.empty()) {
            Mat img_show = img.clone();
            double start = cv::getTickCount();
            std::vector<cv::Rect> face_rects;

            std::vector<std::vector<cv::Point2f>> key_pts;
            face_rects = face_detect->GetFaces(img, key_pts); // 获取人脸框位置同时获取每张脸的5个关键点

            for (int i = 0; i < face_rects.size(); i++)
            {
                face_detect->DrawFaceRect(img_show, face_rects.at(i));
            }
            cv::imshow("img_show", img_show);
            if (cv::waitKey(5) == 27)
            {
                break;
            }
        }
    }
    vision::destroyVisionFaceDetect(face_detect);
}
```

----
### 2.&nbsp;&nbsp;关键点定位和归一化
```c++
namespace vision {
    class VISION_API VisionFaceAlignment
    {
    public:
        /* 默认析构函数 */
        virtual ~VisionFaceAlignment() {}

        /* 初始化，输入参数文件param_path和所要使用的GPU编号device_id */
        virtual bool Init(std::string param_path = "models/face_align.json", int device_id = 0) = 0;

        /* 输入一张图像img和人脸框face_rect，输出人脸的关键点位置 */
        virtual std::vector<cv::Point2f> GetKeyPoints(const cv::Mat& img, const cv::Rect& face_rect) = 0;

        /* 批量获取人脸的关键点位置 */
        virtual std::vector<std::vector<cv::Point2f>> GetKeyPoints(const cv::Mat& img, const std::vector<cv::Rect>& face_rects) = 0;

        /* 输入一张图像img和人脸框face_rect，输出一张归一化人脸（180x220）。若padding是true，则在图像变换的时候采用周边的像素进行补充 */
        virtual cv::Mat GetAlignedFace(const cv::Mat& img, const cv::Rect& face_rect, bool padding = false) = 0;

        /* 批量获取归一化人脸 */
        virtual std::vector<cv::Mat> GetAlignedFace(const cv::Mat& img, const std::vector<cv::Rect>& face_rect, bool padding = false) = 0;

        /* 输入一张图像img和人脸关键点key_pts，输出一张归一化人脸 */
        virtual cv::Mat GetAlignedFace(const cv::Mat& img, const std::vector<cv::Point2f>& key_pts, bool padding = false) = 0;

        /* 批量获取归一化人脸 */
        virtual std::vector<cv::Mat> GetAlignedFace(const cv::Mat& img, const std::vector<std::vector<cv::Point2f>>& key_pts, bool padding = false) = 0;
    };

    /* 构造对象，新增变量device_id，默认使用GPU 0 */
    VISION_API VisionFaceAlignment* instantiateVisionFaceAlignment(int device_id = 0);

    /* 销毁对象 */
    VISION_API void destroyVisionFaceAlignment(VisionFaceAlignment* ptr);
}
```
示例代码
```c++
void CameraTest()
{
    VisionFaceDetect* face_detect = instantiateVisionFaceDetect();
    VisionFaceAlignment* face_align = instantiateVisionFaceAlignment();

    cv::VideoCapture capture(0);
    Mat img;

    while (true)
    {
        capture >> img;
        std::vector<std::vector<cv::Point2f>> key_pts;
        std::vector<cv::Rect> face_rects = face_detect->GetFaces(img, key_pts);
        // warning: CPU模式下，人脸检测不会返回关键点，需要通过VisionFaceAlignment获取关键点
        if (key_pts.size() == 0)
        {
            key_pts = face_align->GetKeyPoints(img, face_rects);
        }

        for (int i = 0; i < key_pts.size(); i++)
        {
            for (int j = 0; j < key_pts.at(i).size(); j++)
            {
                cv::circle(img, key_pts.at(i).at(j), 2, cv::Scalar(255, 0, 0), 2);
            }
        }

        std::vector<cv::Mat> norm_faces = face_align->GetAlignedFace(img, key_pts);
        if (norm_faces.size() > 0)
        {
            cv::imshow("norm_face", norm_faces[0]);
        }

        cv::imshow("alignment result", img);

        if (cv::waitKey(5) == 27)
        {
            break;
        }
    }

    vision::destroyVisionFaceDetect(face_detect);
    vision::destroyVisionFaceAlignment(face_align);
}
```

----
### 3.&nbsp;&nbsp;人脸特征提取与比对
```c++
namespace vision {
    class VISION_API VisionFaceFeature
    {
    public:
        /* 默认析构函数 */
        virtual ~VisionFaceFeature() {}

        /* 初始化，输入参数文件param_path和所要使用的GPU编号device_id */
        virtual bool Init(std::string param_path = "models/face_feature.json", int device_id = 0) = 0;

        /* 初始化，输入参数文件param_path和所要使用的GPU编号device_id，针对V3的模型【推荐使用】 */
        virtual bool InitV3(std::string param_path = "face_feature_v3.param", int device_id = 0) = 0;

        /* 获取输入归一化人脸图像img的特征 */
        virtual std::vector<float> GetFeature(const cv::Mat& img) = 0;

        /* 批量获取归一化人脸图像特征 */
        virtual std::vector<std::vector<float>> GetFeature(const std::vector<cv::Mat>& imgs) = 0;

        /* 获取两张人脸特征的相似度 */
        virtual float GetScore(const std::vector<float>& fea1, const std::vector<float>& fea2, bool origin_score = false) = 0;
    };

    /* 构造对象， 默认使用GPU 0 */
    VISION_API VisionFaceFeature* instantiateVisionFaceFeature(int device_id = 0);

    /* 销毁对象 */
    VISION_API void destroyVisionFaceFeature(VisionFaceFeature* ptr);
}
```
人脸比对示例代码
```c++
int main(int argc, char** argv) {
    //vision::SetLicenseFile("d:/dip/license.lic");
    if (argc == 4) {
        std::string model_name = std::string(argv[1]);
        std::string image1_name       = std::string(argv[2]);
        std::string image2_name       = std::string(argv[3]);

        vision::VisionFaceDetect *   face_detect    = vision::instantiateVisionFaceDetect();    // 包含默认初始化参数，无需再次初始化
        face_detect->SetMaxWidthGlobal(480);
        face_detect->SetMinFaceSize(80);
        vision::VisionFaceAlignment *face_alignment = vision::instantiateVisionFaceAlignment(); // 包含默认初始化参数，无需再次初始化
        vision::VisionFaceFeature *  face_feature   = vision::instantiateVisionFaceFeature();   // 针对不同的使用场景，需要用特定的模型进行初始化
        face_feature->InitV3(model_name);

        cv::Mat img1 = cv::imread(image1_name);
        cv::Mat img2 = cv::imread(image2_name);

        // Perform Face Detect First
        std::vector<vision::VisionFace> faces1 = face_detect->GetMaximumFace(img1);
        if (faces1.size() < 1) {
            std::cout << "Failed to detect a face in " << image1_name << ", exit." << std::endl;
            exit(-1);
        } else {
            std::cout << "Face rectangle in " << image1_name << " is: " << faces1[0].bbox << std::endl;
        }

        std::vector<vision::VisionFace> faces2 = face_detect->GetMaximumFace(img2);
        if (faces2.size() < 1) {
            std::cout << "Failed to detect a face in " << image2_name << ", exit." << std::endl;
            exit(-1);
        } else {
            std::cout << "Face rectangle in " << image2_name << " is: " << faces2[0].bbox << std::endl;
        }

        // Face Alignment
        cv::Mat norm_face1 = face_alignment->GetAlignedFace(img1, faces1.at(0).key_pts);
        cv::Mat norm_face2 = face_alignment->GetAlignedFace(img2, faces2.at(0).key_pts);

        // Extract Face Feature
        std::vector<float> fea1 = face_feature->GetFeature(norm_face1);
        std::vector<float> fea2 = face_feature->GetFeature(norm_face2);

        // Feature Comapre
        std::cout << "Similarity: " << face_feature->GetScore(fea1, fea2) << std::endl;

        vision::destroyVisionFaceDetect(face_detect);
        vision::destroyVisionFaceAlignment(face_alignment);
        vision::destroyVisionFaceFeature(face_feature);
    } else {
        std::string cmd = std::string(argv[0]);
        std::cout << "Usage: " << cmd << " model_name image1 image2" << std::endl;
    }
    return 0;
}
```

### 使用建议及注意事项
- 根据业务场景使用`SetMinFaceSize`接口设置合理的最小人脸大小，以获取更快的人脸检测速度
- 根据业务场景使用`SetMaxWidthGlobal`接口设置合理的检测图像最长边大小，以获取更快的人脸检测速度
- 在嵌入式平台使用该模块，建议同比例减小`MinFaceSize`和`MaxWidthGlobal`，减少IO，加速人脸检测
 - 两张人脸的相似度结果（比分0.5以上，说明像；比分0.65以上，比较像；比分0.75以上，非常像；该值的设定具体要参照业务场景）

----
##  Contact
chendd14 <chendd14@163.com>
<div class="footer">
Copyright(c) chendd14
</div>
