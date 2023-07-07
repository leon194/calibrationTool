#pragma once

#include <memory>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

namespace b3di {

  // A simple header only class to encapsulate compressed and uncompressed cv::Mat.
  class TImage
  {
  public:
    using CompressedData = std::vector<uchar>;
    using CompressedDataPtr = std::shared_ptr<CompressedData>;
    using EncodingParameters = std::vector<int>;

    TImage()
    : m_image(), m_compressedDataPtr()
    {}

    ~TImage()
    {}

    explicit TImage(cv::Mat&& image)
    : m_image(std::move(image)), m_compressedDataPtr()
    {}

    explicit TImage(const cv::Mat& image)
    : m_image(image), m_compressedDataPtr()
    {}

    TImage(const cv::Mat& image, bool compress)
    : TImage(image, compress, ".png", {})
    {}

    TImage(const cv::Mat& image, bool compress, const cv::String& extension, const EncodingParameters& encodingParameters)
    : m_image(), m_compressedDataPtr()
    {
      if (!compress) {
        m_image = image;
      } else if (!image.empty()) {
        m_compressedDataPtr = std::make_shared<CompressedData>();
        if (!cv::imencode(extension, image, *m_compressedDataPtr, encodingParameters)) {
          throw std::runtime_error("TImage: Failed to compress image!");
        }
      }
    }

    explicit TImage(CompressedData&& compressedData)
    : m_image(), m_compressedDataPtr(compressedData.empty() ? nullptr : std::make_shared<CompressedData>(std::move(compressedData)))
    {}

    explicit TImage(const CompressedData& compressedData)
    : m_image(), m_compressedDataPtr(compressedData.empty() ? nullptr : std::make_shared<CompressedData>(compressedData))
    {}

    cv::Mat getImage() const
    {
      if (nullptr == m_compressedDataPtr) {
        return m_image;
      }

      const auto image = cv::imdecode(*m_compressedDataPtr, cv::IMREAD_UNCHANGED);
      if (image.empty()) {
        throw std::runtime_error("TImage: Failed to decompress image!");
      }
      return image;
    }

    const CompressedDataPtr& getCompressedDataPtr() const
    {
        return m_compressedDataPtr;
    }

    std::size_t getDataSize() const
    {
      if (nullptr != m_compressedDataPtr) {
        return m_compressedDataPtr->size();
      }

      // https://stackoverflow.com/questions/26441072/finding-the-size-in-bytes-of-cvmat
      return m_image.empty() ? 0 : m_image.step[0] * m_image.rows;
    }

    bool compressed() const
    {
      return nullptr != m_compressedDataPtr;
    }

    bool empty() const
    {
      return nullptr == m_compressedDataPtr && m_image.empty();
    }

    void release()
    {
      m_image.release();
      m_compressedDataPtr.reset();
    }

  private:
    cv::Mat m_image;
    CompressedDataPtr m_compressedDataPtr;
  };

} // namespace b3di
