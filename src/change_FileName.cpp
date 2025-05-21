#include <iostream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <heif.h>

namespace fs = std::filesystem;

cv::Mat heif_to_mat(const std::string& path) {
    heif_context* ctx = heif_context_alloc();
    heif_error err = heif_context_read_from_file(ctx, path.c_str(), nullptr);
    if (err.code != heif_error_Ok) {
        std::cerr << "Failed to read HEIC file: " << path << std::endl;
        heif_context_free(ctx);
        return {};
    }

    heif_image_handle* handle;
    heif_context_get_primary_image_handle(ctx, &handle);

    heif_image* img;
    heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, nullptr);

    int width = heif_image_get_width(img, heif_channel_interleaved);
    int height = heif_image_get_height(img, heif_channel_interleaved);

    const uint8_t* data = heif_image_get_plane_readonly(img, heif_channel_interleaved, nullptr);
    cv::Mat image(height, width, CV_8UC3, (void*)data);

    cv::Mat copied;
    image.copyTo(copied);

    heif_image_release(img);
    heif_image_handle_release(handle);
    heif_context_free(ctx);

    return copied;
}

int main() {
    std::string input_dir = "../test_piece/";
    int target_width = 640;

    int count = 0;
    for (const auto& entry : fs::directory_iterator(input_dir)) {
        if (entry.is_regular_file()) {
            std::string path = entry.path().string();
            std::string ext = entry.path().extension().string();
            if (ext == ".HEIC" || ext == ".heic") {
                cv::Mat image = heif_to_mat(path);
                if (image.empty()) continue;

                int w = image.cols, h = image.rows;
                float ratio = target_width / static_cast<float>(w);
                int new_height = static_cast<int>(h * ratio);

                cv::Mat resized;
                cv::resize(image, resized, cv::Size(target_width, new_height));

                std::string new_filename = input_dir + "image_" + std::to_string(count++) + ".jpg";
                std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 95};
                cv::imwrite(new_filename, resized, params);

                std::cout << "Saved: " << new_filename << std::endl;
            }
        }
    }

    return 0;
}
