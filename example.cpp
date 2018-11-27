#include "VisionFaceDetect.h"
#include "VisionFaceFeature.h"
#include "VisionFaceAlignment.h"

#include <stdio.h>
#include <string>
#include <iostream>

int EfficiencyTest(std::string model_name, std::string image_name) {
    vision::VisionFaceDetect * face_detect    = vision::instantiateVisionFaceDetect();
    face_detect->SetMaxWidthGlobal(480);
    face_detect->SetMinFaceSize(80);
    vision::VisionFaceAlignment *face_alignment = vision::instantiateVisionFaceAlignment();
    vision::VisionFaceFeature *face_feature   = vision::instantiateVisionFaceFeature();
    if(model_name != "") {
        if (!face_feature->InitV3(model_name)) {
            std::cout << "Failed to init face feature, please check model path." << std::endl;
            exit(-1);
        }
    }

    cv::Mat img = cv::imread(image_name);
    double start, stop, time;
    for(int i=0; i<100; i++) {
        // face detect
        start = (double)cvGetTickCount();
        std::vector<vision::VisionFace> faces = face_detect->GetMaximumFace(img);
        stop  = (double)cvGetTickCount();
        time  = (stop - start) / (cvGetTickFrequency() * 1000);
        std::cout << "face detect time cost: " << time << " ms." << std::endl;

        start = (double)cvGetTickCount();
        cv::Mat norm_face = face_alignment->GetAlignedFace(img, faces[0].key_pts);
        stop  = (double)cvGetTickCount();
        time  = (stop - start) / (cvGetTickFrequency() * 1000);
        std::cout << "face alignment time cost: " << time << " ms." << std::endl;

        start = (double)cvGetTickCount();
        std::vector<float> feature_tmp = face_feature->GetFeature(norm_face);;
        stop  = (double)cvGetTickCount();
        time  = (stop - start) / (cvGetTickFrequency() * 1000);
        std::cout << "face feature time cost: " << time << " ms." << std::endl;
    }

    vision::destroyVisionFaceDetect(face_detect);
    vision::destroyVisionFaceAlignment(face_alignment);
    vision::destroyVisionFaceFeature(face_feature);
    return 0;
}

int main(int argc, char** argv) {
    //vision::SetLicenseFile("d:/dip/license.lic");
    if (argc == 3) {
        std::string model_name = std::string(argv[1]);
        std::string image_name       = std::string(argv[2]);
        EfficiencyTest(model_name, image_name);
    } else if (argc == 4) {
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