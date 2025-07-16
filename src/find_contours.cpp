#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    cv::Mat img = cv::imread("test.jpg");
    if (img.empty()) {
        std::cerr << "画像の読み込みに失敗しました" << std::endl;
        return -1;
    }

    // グレースケール化＆ぼかし＆2値化
    // cv::Mat gray, blurred, thresh, pre_img1, pre_img2, pre_img3;
    // cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    // cv::GaussianBlur(gray, blurred, cv::Size(15, 15), 0);
    // // ヒストグラム均一化
    // cv::equalizeHist(gray, pre_img1);

    // // CLAHEコントラスト調整
    // cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    // // CLAHEを画像に適用
    // clahe->apply(gray, pre_img2);

    // cv::imshow("Hist",pre_img1);
    // cv::imshow("CLAHE",pre_img2);
    // cv::waitKey(0);
    // cv::destroyAllWindows();

    // cv::Mat thresh_otsu,thresh_adap;
    // cv::adaptiveThreshold(pre_img1, thresh_adap, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 31, 50);
    // cv::threshold(pre_img1, thresh_otsu, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    // cv::imshow("Adap", thresh_adap);
    // cv::imshow("Otsu", thresh_otsu);
    // cv::waitKey(0);
    // cv::destroyAllWindows();
    // // 輪郭抽出
    // std::vector<std::vector<cv::Point>> contours;
    // cv::findContours(thresh_adap, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // if (contours.empty()) {
    //     std::cerr << "輪郭が見つかりません" << std::endl;
    //     return -1;
    // }

    // // 最大の輪郭を選択
    // size_t max_idx = 0;
    // double max_area = 0;
    // for (size_t i = 0; i < contours.size(); ++i) {
    //     double area = cv::contourArea(contours[i]);
    //     if (area > max_area) {
    //         max_area = area;
    //         max_idx = i;
    //     }
    // }

    // // 輪郭近似（四角形化）
    // std::vector<cv::Point> approx;
    // cv::approxPolyDP(contours[max_idx], approx, cv::arcLength(contours[max_idx], true) * 0.02, true);

    // // 元画像に輪郭を描画
    // cv::Mat contour_img = img.clone();
    // cv::drawContours(contour_img, contours, -1, cv::Scalar(0, 255, 0), 2); // 緑色で全ての輪郭

    // // 表示
    // cv::imshow("輪郭", contour_img);
    // cv::waitKey(0);
    // cv::destroyAllWindows();

    // if (approx.size() != 4) {
    //     std::cerr << "四角形が検出できませんでした（検出点数: " << approx.size() << "）" << std::endl;
    //     return -1;
    // }

    // // 点の並び順を正しく（左上→右上→右下→左下）にする
    // std::vector<cv::Point2f> src_pts(4);
    // std::sort(approx.begin(), approx.end(), [](cv::Point a, cv::Point b) { return a.y < b.y; });
    // if (approx[0].x < approx[1].x) {
    //     src_pts[0] = approx[0];  // 左上
    //     src_pts[1] = approx[1];  // 右上
    // } else {
    //     src_pts[0] = approx[1];
    //     src_pts[1] = approx[0];
    // }
    // if (approx[2].x < approx[3].x) {
    //     src_pts[3] = approx[2];  // 左下
    //     src_pts[2] = approx[3];  // 右下
    // } else {
    //     src_pts[3] = approx[3];
    //     src_pts[2] = approx[2];
    // }

    // 1. 入力と src_pts（左上 → 左下 → 右下 → 右上）
    cv::Mat input_image = cv::imread("test.jpg");
    std::vector<cv::Point2f> src_pts = {
        {3, 3},        // 左上
        {26, 212},     // 左下
        {232, 237},    // 右下
        {236, 0}       // 右上
    };

    // 1. 中心を計算
    cv::Point2f center(0, 0);
    for (const auto& pt : src_pts) center += pt;
    center *= 1.0f / src_pts.size();

    // 2. 縮小比率または距離
    float shrink_ratio = 0.05f;  // 5%だけ中心に縮める（または↓固定距離で縮める）
    float shrink_px = 10.0f;

    // 3. 各点を中心に向かって縮める
    for (auto& pt : src_pts) {
        cv::Point2f direction = center - pt;
        float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (len > 1e-2) { // 0除算回避
            //  direction *= shrink_px / len;
            direction *= shrink_ratio;  // 縮めたい距離分だけ縮小ベクトルを作る
            pt += direction;
        }
    }
    // 2. アスペクト比から目的サイズ決定
    float widthA = cv::norm(src_pts[0] - src_pts[3]);  // 上辺
    float widthB = cv::norm(src_pts[1] - src_pts[2]);  // 下辺
    float width = (widthA + widthB) / 2.0;

    float heightA = cv::norm(src_pts[0] - src_pts[1]);  // 左辺
    float heightB = cv::norm(src_pts[3] - src_pts[2]);  // 右辺
    float height = (heightA + heightB) / 2.0;

    int warped_width = static_cast<int>(std::round(width));
    int warped_height = static_cast<int>(std::round(height));

    // 3. 射影先の座標（左上 → 左下 → 右下 → 右上）
    std::vector<cv::Point2f> dst_pts = {
        {0.0f, 0.0f},
        {0.0f, static_cast<float>(warped_height - 1)},
        {static_cast<float>(warped_width - 1), static_cast<float>(warped_height - 1)},
        {static_cast<float>(warped_width - 1), 0.0f}
    };

    // 4. 射影変換
    cv::Mat M = cv::getPerspectiveTransform(src_pts, dst_pts);
    cv::Mat warped;
    cv::warpPerspective(input_image, warped, M, cv::Size(warped_width, warped_height));

    // 5. 正方形キャンバスに中央配置（黒背景）
    int square_size = std::max(warped_width, warped_height);
    cv::Mat square_output(square_size, square_size, warped.type(), cv::Scalar(0, 0, 0)); // 黒背景

    int x_offset = (square_size - warped.cols) / 2;
    int y_offset = (square_size - warped.rows) / 2;

    // ROIでコピー
    warped.copyTo(square_output(cv::Rect(x_offset, y_offset, warped.cols, warped.rows)));

    // 6. 表示・保存
    cv::imshow("Square", square_output);
    cv::imwrite("square_result.png", square_output);

    return 0;
}
