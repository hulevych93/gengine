#include <brokers/PersistencyBroker.h>

#include <boost/signals2.hpp>
#include <unordered_map>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <appconfig/AppConfig.h>
#include <brokers/WorkerBroker.h>
#include <core/Encoding.h>

#include <filesystem/FileSearcher.h>
#include <filesystem/Filesystem.h>
#include <shared-services/SharedServiceExport.h>

#include <serialization/Deserializer.h>
#include <serialization/ISerializable.h>
#include <serialization/Serializer.h>

#include <core/Blob.h>
#include <core/Logger.h>
#include <json/JSON.h>

namespace Gengine {

using namespace Serialization;
using namespace Services;

namespace Services {

std::shared_ptr<Blob> readBlob(const std::wstring& file) {
  namespace fs = boost::filesystem;

  fs::path filePath(file);

  if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
    boost::filesystem::ifstream fileData(filePath, std::ios::binary);

    fileData.seekg(0, std::ios::end);
    auto size = fileData.tellg();
    fileData.seekg(0, std::ios::beg);

    auto blob = std::make_shared<Blob>(size);
    fileData.read(reinterpret_cast<char*>(blob->GetData()), size);
    return blob;
  }

  return std::shared_ptr<Blob>();
}

bool writeBlob(const std::wstring& file, const std::shared_ptr<Blob>& blob) {
  boost::filesystem::ofstream fileData(file, std::ios::binary);
  fileData.write(reinterpret_cast<const char*>(blob->GetData()),
                 blob->GetSize());
  fileData.close();

  return true;
}

using persistency_read_signal =
    boost::signals2::signal<void(const IPersistency& persistency)>;
using persistency_write_signal =
    boost::signals2::signal<void(IPersistency& persistency)>;

class PersistencyBroker : public IPersistencyBroker {
 public:
  void Configure(const std::string& directory) override {
    m_directory = directory;
    Filesystem::CreateFolder(utf8toWchar(m_directory));

    try {
      auto persistencyFiles =
          FileSeacher::Search(directory, L".*\\.persistency");
      for (const auto& persistencyFile : persistencyFiles) {
        auto persistency = std::make_unique<PersistencyContext>();
        persistency->file = toUtf8(persistencyFile->FileName());
        persistency->data = readBlob(persistencyFile->FilePath().wstring());
        signals.emplace(
            std::make_pair(toUtf8(persistencyFile->FileNameWithoutExtension()),
                           std::move(persistency)));
      }
    } catch (std::exception& ex) {
      GLOG_WARNING_INTERNAL("Failed to configure persistency: %s", ex.what());
    }
  }

  void Deconfigure() override {
    for (const auto& signal : signals) {
      auto& context = signal.second;
      context->Write(m_directory);
    }
    signals.clear();
  }

  connection Subscribe(const std::string& id,
                       IPersistencyClient& client) override {
    connection conn;
    auto iter = signals.find(id);
    if (iter != signals.end()) {
      auto& context = iter->second;
      conn = context->Subscribe(client);
    } else {
      auto persistency = std::make_unique<PersistencyContext>();
      persistency->file = id + ".persistency";
      conn = persistency->Subscribe(client);
      signals.emplace(std::make_pair(id, std::move(persistency)));
    }
    return conn;
  }

  void Load(const std::string& id) override {
    auto iter = signals.find(id);
    if (iter != signals.end()) {
      auto& context = iter->second;
      return context->Read();
    }
  }

  void Delete(const std::string& id) override {
    auto iter = signals.find(id);
    if (iter != signals.end()) {
      auto& context = iter->second;
      return context->Delete(m_directory);
    }
  }

  void Store(const std::string& id) const override {
    auto iter = signals.find(id);
    if (iter != signals.end()) {
      auto& context = iter->second;
      return context->Write(m_directory);
    }
  }

 private:
  struct PersistencyContext : public IPersistency {
   public:
    connection Subscribe(IPersistencyClient& client) {
      connection conns;
      conns.emplace_back(
          read_signal.connect([&client](const IPersistency& persistency) {
            client.OnPersistency(persistency);
          }));
      conns.emplace_back(
          write_signal.connect([&client](IPersistency& persistency) {
            const_cast<const IPersistencyClient&>(client).OnPersistency(
                persistency);
          }));
      return conns;
    }

    void Delete(const std::string& directory) {
      const auto fullPath = Filesystem::CombinePath(directory, file);
      Filesystem::DeleteExistingFile(utf8toWchar(fullPath));
    }

    void Read() const { read_signal(*this); }

    void Write(const std::string& directory) {
      write_signal(*this);
      if (data) {
        writeBlob(utf8toWchar(Filesystem::CombinePath(directory, file)), data);
      }
    }

    void operator()(const Serialization::ISerializable& object) override {
      Serializer serializer;
      object.Serialize(serializer);
      data = serializer.GetBlob();
    }

    void operator()(Serialization::ISerializable& object) const override {
      if (data) {
        Deserializer deserializer(*data);
        object.Deserialize(deserializer);
      }
    }

    void operator()(const JSON::IJsonSerializable& object) override {
      JSON::Value value;
      JSON::InputValue input(value);
      input << object;

      std::string str;
      value.Serialize(str);

      data = std::make_shared<Blob>(str.data(), str.size());
    }

    void operator()(JSON::IJsonSerializable& object) const override {
      if (data) {
        JSON::Value root;
        std::string str(reinterpret_cast<char*>(data->GetData()),
                        data->GetSize());
        root.Deserialize(str);
        object.Deserialize(root.ToObject());
      }
    }

    std::string file;
    std::shared_ptr<Blob> data;
    persistency_read_signal read_signal;
    persistency_write_signal write_signal;
  };

  std::map<std::string, std::unique_ptr<PersistencyContext>> signals;
  std::string m_directory;
};

EXPORT_GLOBAL_SHARED_SERVICE(PersistencyBroker)

}  // namespace Services
}  // namespace Gengine
