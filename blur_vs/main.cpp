#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <vector>
#include <iomanip>
#include <random>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>


class simpleTimer {
private:
    std::chrono::time_point<std::chrono::steady_clock> start;
public:
    simpleTimer() {
        start = std::chrono::high_resolution_clock::now();
    }
    ~simpleTimer() {
        auto finish = std::chrono::high_resolution_clock::now();
        std::cout << "Time taken " << std::chrono::duration<float>(finish - start).count() << " s" << std::endl;
    }
};

void createMatrix(std::vector<std::vector<int>>& matrix, int w, int h) {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, 256);

    matrix.resize(w);
    for (int i = 0; i < w; ++i) {
        matrix[i].resize(h);
        for (int j = 0; j < h; ++j) {
            matrix[i][j] = dist(mt);
        }
    }
}

void printMatrix(std::vector<std::vector<int>>& matrix) {
    for (uint32_t i = 0; i < matrix.size(); ++i) {
        for (uint32_t j = 0; j < matrix[i].size(); ++j) {
            std::cout << std::setw(5) << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

void blur(std::vector<std::vector<int>>& matrix, int blur_radius)
{
    uint32_t h = matrix.size();
    uint32_t w = matrix[0].size();

    std::vector<std::vector<int>> result = matrix;

    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            int top_border = (i < blur_radius) ? i : (i-blur_radius);
            int left_border = (j < blur_radius) ? j : (j-blur_radius);
            int right_border = ((w - j) < blur_radius) ? w : (j + blur_radius);
            int bot_border = ((h - i) < blur_radius) ? h : (i + blur_radius);

            long sum = 0;
            int total_num = (bot_border - top_border) * (right_border - left_border);
            for (int k = top_border; k < bot_border; ++k) {
                for (int l = left_border; l < right_border; ++l) {
                    sum += matrix[k][l];
                }
            }
            result[i][j] = sum / total_num;
        }
    }

    matrix = result;
    /*uint32_t wsize = bottom_limit - top_limit;
    * 
    temp_matrix.resize(wsize);
    for (uint32_t i = 0; i < wsize; ++i) {
        temp_matrix[i].resize(h);
        for (uint32_t j = 0; j < h; ++j)
            temp_matrix[i][j] = 0;
    }
    int blur_border = (blur_radius - 1) / 2;
    if (top_limit >= blur_border) {
        for (uint32_t i = blur_border; i < bottom_limit - blur_border; ++i) {
            for (uint32_t j = blur_border; j < h - blur_border; ++j) {
                temp_matrix[i][j] = 0;
                for (uint32_t k = (-1) * blur_border; k < blur_border; ++k) {
                    for (uint32_t l = (-1) * blur_border; l < blur_border; ++l) {
                        temp_matrix[i][j] += matrix[top_limit + i + k][j + l];
                    }
                }
                temp_matrix[i][j] /= (blur_radius * blur_radius);
            }
        }
    }*/
}

int main() {
    std::vector<std::vector<int>> new_matrix;

    std::string image_path = cv::samples::findFile("../image_input/cat.jpg");
    cv::Mat image = cv::imread(image_path, cv::IMREAD_COLOR);
    cv::Mat image_channels[3];
    cv::Mat helper[3];
    cv::split(image, helper);
    for (int i = 0; i < 3; ++i) {
        helper[i].convertTo(image_channels[i], CV_32F);
    }
    
    new_matrix.resize(image_channels[0].rows);
    for (int i = 0; i < image_channels[0].rows; ++i) {
        new_matrix[i].resize(image_channels[0].cols);
        for (int j = 0; j < image_channels[0].cols; ++j) {
            new_matrix[i][j] = image_channels[0].at<int>(i, j);
        }
    }

    
    //createMatrix(new_matrix, 1024, 1280);
    std::cout << "Generated matrix" << std::endl;
    //printMatrix(new_matrix);

    std::cout << "Input blur radius" << std::endl;
    int blur_radius;
    std::cin >> blur_radius;
    {
        simpleTimer st;
        std::vector<std::thread> thread_vector;
        blur(new_matrix, blur_radius);
        /*uint32_t thread_amount = new_matrix.size() / blur_radius;
        const auto core_amount = std::thread::hardware_concurrency(); // TODO: limit amount of threads in realtion with amount of CPU cores.
        uint32_t top_limit, bottom_limit;
        thread_vector.reserve(thread_amount);
        std::cout << thread_amount << " threads created" << std::endl;
        top_limit = 0;
        bottom_limit = top_limit + blur_radius;
        std::vector<std::vector<std::vector<int>>> temp_matrix;
        temp_matrix.resize(thread_amount);
        for (int i = 0; i < thread_amount; ++i) {
            thread_vector.emplace_back(std::thread(blur, std::ref(new_matrix), std::ref(temp_matrix[i]), blur_radius, top_limit, bottom_limit));
            printMatrix(new_matrix);
            top_limit = bottom_limit;
            if (((new_matrix.size() - bottom_limit) / blur_radius) < 2) {
                bottom_limit = new_matrix.size();
            }
            else bottom_limit += blur_radius;
        }
        for (auto& t : thread_vector) {
            t.join();
        }
        new_matrix = temp_matrix[0];
        for (uint32_t i = 1; i < thread_amount; ++i) {
            new_matrix.insert(new_matrix.end(), temp_matrix[i].begin(), temp_matrix[i].end());
        }*/
    }
    std::cout << "Blurred result" << std::endl;
    //printMatrix(new_matrix);

    /*
    std::string image_path = cv::samples::findFile("../image_input/cat.jpg");
    cv::Mat image = cv::imread(image_path, cv::IMREAD_COLOR);
    cv::String window_name = "Blur project";

    std::cout << image.total() << std::endl;

    cv::Mat image_channels[3];
    cv::split(image, image_channels);

    cv::namedWindow(window_name); // Create a window
    cv::imshow(window_name, image); // Show our image inside the created window.
    cv::waitKey(0); // Wait for any keystroke in the window
    cv::destroyWindow(window_name); //destroy the created window
    */
    return 0;
}
