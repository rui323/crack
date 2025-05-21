#include<iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include<fstream>

cv::Mat padToSize(const cv::Mat& img, int target_width, int target_height) {
    int top = (target_height - img.rows) / 2;
    int bottom = target_height - img.rows - top;
    int left = (target_width - img.cols) / 2;
    int right = target_width - img.cols - left;

    cv::Mat padded;
    cv::copyMakeBorder(img, padded, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
    return padded;
}

int main(int argc, char* argv[]){
    std::vector<cv::Mat> image_list;
    std::vector<cv::Mat> image_blur;
    std::vector<cv::Mat> images_adt;

    int max_w = 0, max_h = 0;

    std::string method = argv[3];  // ← ここで一度string化しておく

    // 読み込みと前処理---------------------------
    for( int i = 0 ; i < 11; i++ ){
        std::string path = "../trimming_image/image_"+std::to_string(i)+".jpg";
        cv::Mat image = cv::imread(path,cv::IMREAD_GRAYSCALE );
        image_list.push_back(image);
        max_w = std::max(max_w, image.cols);
        max_h = std::max(max_h, image.rows);

        // 入力画像をぼかす　ガウシアンフィルタ(GaussianBlur)
        cv::Mat blur_image;
        // cv::GaussianBlur(image, blur_image, cv::Size(3,3),0.1);
        // cv::medianBlur(image, blur_image, 3);
        cv::bilateralFilter(image, blur_image, 9 , 75, 75);
        image_blur.push_back(blur_image);

        cv::Mat dst_adaptive;
        if(method == "M")cv::adaptiveThreshold(image, dst_adaptive, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, std::stoi(argv[1]), std::stod(argv[2]));
        else cv::adaptiveThreshold(image, dst_adaptive, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, std::stoi(argv[1]), std::stod(argv[2]));
        images_adt.push_back(dst_adaptive);
        std::cout << "Save image: " << dst_adaptive.size() << " " <<images_adt.size()<< std::endl;
    }

    std::vector<cv::Mat> image_list_r;
    std::vector<cv::Mat> image_blur_r;
    std::vector<cv::Mat> images_adt_r;
    
    for (auto& img : image_list) {// 画像サイズを最大に合わせる、足りない分は黒画像で埋める
        cv::Mat image = padToSize(img, max_w, max_h);
        image_list_r.push_back(image);
    }

    for (auto& img : images_adt) {// 画像サイズを最大に合わせる、足りない分は黒画像で埋める
        cv::Mat image = padToSize(img, max_w, max_h);
        images_adt_r.push_back(image);
    }

    for (auto& img : image_blur) {// 画像サイズを最大に合わせる、足りない分は黒画像で埋める
        cv::Mat image = padToSize(img, max_w, max_h);
        image_blur_r.push_back(image);
    }
    // -----------------------------------

    // 線検出------------------------------cv::HoughLinesは　判定にx軸となす角もみる　よって45度の直線を反映させるためtheta=45
    std::vector<cv::Mat> images_result;
    std::string path;
    if(method=="M") path = "../results/result_M.txt";
    else path = "../results/result_G.txt";
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "ファイルを開けませんでした: " << path << std::endl;
        return 1;
    }
    for(int i = 0; i < 11; i++){
        // Hough変換で直線検出
        std::vector<cv::Vec4i> lines;
        // double threshold = images_adt[std::stoi(argv[3])].size().width * 50 / 374;// 線分として認める最小交差点数
        // double minLineGap = images_adt[std::stoi(argv[3])].size().width * 30 / 374;// 検出する最小の線分の長さ（それ未満は除外）
        // double maxLineGap = images_adt[std::stoi(argv[3])].size().width * 100 / 374;// 線分を連結するときの最大間隔（距離）
        // std::cout << "width: " << images_adt[std::stoi(argv[3])].size().width << ", threshold " << threshold <<  ", min line " << minLineGap << ", max line " << maxLineGap << std::endl;
        // cv::HoughLinesP(images_adt[std::stoi(argv[3])], lines, 1, CV_PI / 180, threshold, minLineGap, maxLineGap);

        double threshold = images_adt[i].size().width * 50 / 374;// 線分として認める最小交差点数
        double minLineGap = images_adt[i].size().width * 30 / 374;// 検出する最小の線分の長さ（それ未満は除外）
        double maxLineGap = images_adt[i].size().width * 100 / 374;// 線分を連結するときの最大間隔（距離）
        std::cout << "width: " << images_adt[i].size().width << ", threshold " << threshold <<  ", min line " << minLineGap << ", max line " << maxLineGap << std::endl;
        cv::HoughLinesP(images_adt[i], lines, 1, CV_PI / 180, threshold, minLineGap, maxLineGap);

        cv::Mat result_image = image_list[i].clone();
        // 線の分類
        for (const auto& line : lines) {
            int x1 = line[0], y1 = line[1];
            int x2 = line[2], y2 = line[3];
            double dx = x2 - x1;
            double dy = y2 - y1;

            double angle = std::atan2(dy, dx) * 180.0 / CV_PI;
            if (angle < 0) angle += 360.0;

            std::string type;
            if ((angle >= 0 && angle <= 5) || (angle >= 175 && angle <= 185)) {
                type = "horizontal";
            } else if ((angle >= 85 && angle <= 95) || (angle >= 265 && angle <= 275)) {
                type = "vertical";
            } else if ((angle >= 40 && angle <= 50) || (angle >= 130 && angle <= 140) ||
                    (angle >= 220 && angle <= 230) || (angle >= 310 && angle <= 320)) {
                type = "diagonal";
            } else {
                type = "other";
            }

            file << "result_" << i <<".jpg, Angle: " << angle << " deg -> " << type << "\n";
         
            // 線を描画（色を角度で分ける）
            cv::Scalar color;
            if (type == "horizontal") color = cv::Scalar(0, 255, 0);      // 緑
            else if (type == "vertical") color = cv::Scalar(255, 0, 0);   // 青
            else if (type == "diagonal") color = cv::Scalar(0, 0, 255);   // 赤
            else color = cv::Scalar(200, 200, 200);                       // グレー

            // cv::line(image_list[std::stoi(argv[3])], cv::Point(x1, y1), cv::Point(x2, y2), color, 2);
            cv::line(result_image, cv::Point(x1, y1), cv::Point(x2, y2), color, 2);
            
        }
        images_result.push_back(result_image);
    }
    file.close();
    // 結果表示
    // cv::imshow("Detected Lines", image_list[std::stoi(argv[3])]);
    // cv::waitKey(0);

    // 画像サイズ変更
    std::vector<cv::Mat> images_result_r;
    for (auto& img : images_result) {// 画像サイズを最大に合わせる、足りない分は黒画像で埋める
        cv::Mat image = padToSize(img, max_w, max_h);
        images_result_r.push_back(image);
    }

    // 2枚一組で横に並べて保存
    for (size_t i = 0; i < image_list.size(); ++i) {
        cv::Mat combined1, combined2,combined3;
        std::vector<cv::Mat> imgs = {
            image_list_r[i],
            image_blur_r[i],
            images_adt_r[i],
            images_result_r[i]
        };
        cv::hconcat(imgs[0],imgs[1], combined1);
        cv::hconcat(imgs[2],imgs[3], combined2);
        cv::vconcat(combined1,combined2,combined3);

        
        std::string filename;
        if(method=="M") filename = "../results/result_M_" + std::to_string(i) + ".jpg";
        else filename = "../results/result_G_" + std::to_string(i) + ".jpg";
        
        cv::imwrite(filename, combined3);
        std::cout << "Saved: " << filename << std::endl;
    }

    



    return 0;
}