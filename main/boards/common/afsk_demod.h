#pragma once

#include <vector>
#include <deque>
#include <string>
#include <memory>
#include <optional>
#include <cmath>
#include "wifi_manager.h"
#include "application.h"

// Hằng số xử lý tín hiệu âm thanh cho cấu hình WiFi qua âm thanh
const size_t kAudioSampleRate = 6400;
const size_t kMarkFrequency = 1800;
const size_t kSpaceFrequency = 1500;
const size_t kBitRate = 100;
const size_t kWindowSize = 64;

namespace audio_wifi_config
{
    // Chức năng chính để nhận thông tin xác thực WiFi thông qua tín hiệu âm thanh
    void ReceiveWifiCredentialsFromAudio(Application *app, WifiManager *wifi_manager, Display *display, 
                                         size_t input_channels = 1);

    /* *
     * Triển khai thuật toán Goertzel để phát hiện tần số đơn
     * Được sử dụng để phát hiện các tần số âm thanh cụ thể trong quá trình giải điều chế AFSK */
    class FrequencyDetector
    {
    private:
        float frequency_;              // Tần số mục tiêu (chuẩn hóa, tức là f / fs)
        size_t window_size_;           // Kích thước cửa sổ để phân tích
        float frequency_bin_;          // Thùng tần số
        float angular_frequency_;      // Tần số góc
        float cos_coefficient_;        // cos(w)
        float sin_coefficient_;        // tội lỗi(w)
        float filter_coefficient_;     // 2 * cos(w)
        std::deque<float> state_buffer_;  // Bộ đệm tròn để lưu trữ S[-1] và S[-2]

    public:
        /* *
         * Trình xây dựng
         * @param tần số Tần số chuẩn hóa (f / fs)
         * @param window_size Kích thước cửa sổ để phân tích */
        FrequencyDetector(float frequency, size_t window_size);

        /* *
         * Đặt lại trạng thái máy dò */
        void Reset();

        /* *
         * Xử lý một mẫu âm thanh
         * @param sample Mẫu âm thanh đầu vào */
        void ProcessSample(float sample);

        /* *
         * Tính biên độ dòng điện
         * @return Giá trị biên độ */
        float GetAmplitude() const;
    };

    /* *
     * Bộ xử lý tín hiệu âm thanh để phát hiện cặp tần số Mark/Space
     * Xử lý tín hiệu âm thanh để trích xuất dữ liệu số bằng phương pháp giải điều chế AFSK */
    class AudioSignalProcessor
    {
    private:
        std::deque<float> input_buffer_;             // Bộ đệm mẫu đầu vào
        size_t input_buffer_size_;                   // Kích thước bộ đệm đầu vào = kích thước cửa sổ
        size_t output_sample_count_;                 // Bộ đếm mẫu đầu ra
        size_t samples_per_bit_;                     // Ngưỡng mẫu trên mỗi bit
        std::unique_ptr<FrequencyDetector> mark_detector_;   // Đánh dấu máy dò tần số
        std::unique_ptr<FrequencyDetector> space_detector_;  // Máy dò tần số không gian

    public:
        /* *
         * Trình xây dựng
         * @param sample_rate Tốc độ lấy mẫu âm thanh
         * @param mark_information Đánh dấu tần số cho kỹ thuật số '1'
         * @param space_number Tần số không gian cho kỹ thuật số '0'
         * @param bit_rate Tốc độ bit truyền dữ liệu
         * @param window_size Phân tích kích thước cửa sổ */
        AudioSignalProcessor(size_t sample_rate, size_t mark_frequency, size_t space_frequency,
                           size_t bit_rate, size_t window_size);

        /* *
         * Xử lý mẫu âm thanh đầu vào
         * @param mẫu Vectơ mẫu âm thanh đầu vào
         * @return Vector của các giá trị xác suất Mark (0,0 đến 1,0) */
        std::vector<float> ProcessAudioSamples(const std::vector<float> &samples);
    };

    /* *
     * Trạng thái máy nhận dữ liệu */
    enum class DataReceptionState
    {
        kInactive,  // Đang chờ tín hiệu bắt đầu
        kWaiting,   // Đã phát hiện khả năng bắt đầu, đang chờ xác nhận
        kReceiving  // Chủ động tiếp nhận dữ liệu
    };

    /* *
     * Bộ đệm dữ liệu để quản lý chuyển đổi dữ liệu âm thanh sang kỹ thuật số
     * Xử lý toàn bộ quá trình từ tín hiệu âm thanh đến dữ liệu văn bản được giải mã */
    class AudioDataBuffer
    {
    private:
        DataReceptionState current_state_;       // Trạng thái tiếp nhận hiện tại
        std::deque<uint8_t> identifier_buffer_;  // Bộ đệm để phát hiện mã định danh bắt đầu/kết thúc
        size_t identifier_buffer_size_;          // Kích thước bộ đệm định danh
        std::vector<uint8_t> bit_buffer_;        // Bộ đệm để lưu trữ dòng bit
        size_t max_bit_buffer_size_;             // Kích thước bộ đệm bit tối đa
        const std::vector<uint8_t> start_of_transmission_;  // Mã định danh bắt đầu truyền
        const std::vector<uint8_t> end_of_transmission_;    // Mã định danh cuối truyền
        bool enable_checksum_validation_;       // Có xác nhận tổng kiểm tra hay không

    public:
        std::optional<std::string> decoded_text; // Đã giải mã thành công dữ liệu văn bản

        /* *
         * Hàm tạo mặc định sử dụng mã định danh bắt đầu và kết thúc được xác định trước */
        AudioDataBuffer();

        /* *
         * Trình xây dựng với các tham số tùy chỉnh
         * @param max_byte_size Kích thước dữ liệu tối đa dự kiến tính bằng byte
         * @param start_identifier Mã định danh bắt đầu truyền
         * @param end_identifier Mã định danh kết thúc truyền
         * @param Enable_checksum Có bật xác thực tổng kiểm tra hay không */
        AudioDataBuffer(size_t max_byte_size, const std::vector<uint8_t> &start_identifier,
                      const std::vector<uint8_t> &end_identifier, bool enable_checksum = false);

        /* *
         * Xử lý dữ liệu xác suất và cố gắng giải mã
         * @param xác suất Vector của xác suất Mark
         * Ngưỡng @param Ngưỡng quyết định để phát hiện bit
         * @return true nếu dữ liệu hoàn chỉnh được nhận và giải mã thành công */
        bool ProcessProbabilityData(const std::vector<float> &probabilities, float threshold = 0.5f);

        /* *
         * Tính tổng kiểm tra cho văn bản ASCII
         * @param text Nhập chuỗi văn bản
         * @return Giá trị tổng kiểm tra (0-255) */
        static uint8_t CalculateChecksum(const std::string &text);

    private:
        /* *
         * Chuyển đổi vectơ bit thành vectơ byte
         * @param bit Vectơ bit đầu vào
         * @return Vectơ byte được chuyển đổi */
        std::vector<uint8_t> ConvertBitsToBytes(const std::vector<uint8_t> &bits) const;

        /* *
         * Xóa tất cả bộ đệm và đặt lại trạng thái */
        void ClearBuffers();
    };

    // Mã định danh truyền bắt đầu và kết thúc mặc định
    extern const std::vector<uint8_t> kDefaultStartTransmissionPattern;
    extern const std::vector<uint8_t> kDefaultEndTransmissionPattern;
}