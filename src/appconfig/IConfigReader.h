#pragma once

namespace Gengine {
namespace AppConfig {

class IConfigReader {
 public:
  virtual ~IConfigReader() = default;
  virtual bool Load() = 0;
  virtual bool Save() const = 0;
};

}  // namespace AppConfig
}  // namespace Gengine