#include <Runtime/Scene/SceneSerializer.hpp>
#include <Runtime/Scene/Scene.hpp>

#include <Common/Serialization/ISerializer.hpp>
#include <Common/Core/Log.hpp>

#include <cstddef>
#include <cstring>
#include <fstream>
#include <vector>

namespace
{
    class VectorSerializer final : public cp::ISerializer
    {
    public:
        void Write(std::span<const std::byte> _bytes) override
        {
            buffer.insert(buffer.end(),
                reinterpret_cast<const uint8_t*>(_bytes.data()),
                reinterpret_cast<const uint8_t*>(_bytes.data()) + _bytes.size());
        }

        [[nodiscard]] const std::vector<uint8_t>& GetBuffer() const { return buffer; }

    private:
        std::vector<uint8_t> buffer;
    };

    class SpanDeserializer final : public cp::IDeserializer
    {
    public:
        explicit SpanDeserializer(const std::vector<uint8_t>& _data)
            : data(_data), cursor(0) {}

        [[nodiscard]] bool Read(std::span<std::byte> _bytes) override
        {
            if (cursor + _bytes.size() > data.size())
                return false;

            std::memcpy(_bytes.data(), data.data() + cursor, _bytes.size());
            cursor += _bytes.size();
            return true;
        }

    private:
        const std::vector<uint8_t>& data;
        size_t cursor;
    };
}

namespace cp::runtime
{
    std::unique_ptr<Scene> SceneSerializer::LoadSceneFromFile(
        const std::filesystem::path& _filePath,
        cp::ILogger* _logger
    )
    {
        auto scene = std::make_unique<Scene>();
        if (!LoadSceneFromFile(*scene, _filePath, _logger))
            return nullptr;
        return scene;
    }

    bool SceneSerializer::LoadSceneFromFile(
        Scene& _scene,
        const std::filesystem::path& _filePath,
        cp::ILogger* _logger
    )
    {
        std::ifstream ifs(_filePath, std::ios::binary | std::ios::ate);
        if (!ifs.is_open())
        {
            if (_logger)
                _logger->Log(CP_LOG_EVENT(cp::ILogger::Error, "Scene Loader", cp::Message::Create("SceneSerializer: failed to open file: {}", _filePath.string())));

            return false;
        }

        std::vector<uint8_t> bytes(static_cast<size_t>(ifs.tellg()));
        ifs.seekg(0);
        ifs.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));

        SpanDeserializer deserializer(bytes);
        if (!_scene.DeserializeFrom(deserializer))
        {
            if (_logger)
                _logger->Log(CP_LOG_EVENT(cp::ILogger::Error, "Scene Loader", cp::Message::Create("SceneSerializer: failed to deserialize: {}", _filePath.string())));

            return false;
        }

        return true;
    }

    bool SceneSerializer::SaveSceneToFile(
        const Scene& _scene,
        const std::filesystem::path& _filePath,
        cp::ILogger* _logger
    )
    {
        VectorSerializer serializer;
        if (!_scene.SerializeTo(serializer))
        {
            if (_logger)
                _logger->Log("SceneSerializer: serialization failed for: " + _scene.GetName(), cp::ILogger::Error);

            return false;
        }

        std::filesystem::create_directories(_filePath.parent_path());
        std::ofstream ofs(_filePath, std::ios::binary | std::ios::trunc);
        if (!ofs.is_open())
        {
            if (_logger)
                _logger->Log("SceneSerializer: failed to write file: " + _filePath.string(), cp::ILogger::Error);

            return false;
        }

        const auto& buf = serializer.GetBuffer();
        ofs.write(reinterpret_cast<const char*>(buf.data()), static_cast<std::streamsize>(buf.size()));
        return ofs.good();
    }
}
