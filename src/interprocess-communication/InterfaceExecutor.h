#pragma once

#include <interprocess-communication/InterprocessCommonDefs.h>

namespace Gengine {
namespace InterprocessCommunication {
class InputParameters;
class OutputParameters;

/**
 * @brief The InterfaceImpl class
 *
 * Common interface fro both InterfaceListener and InterfaceExecutor
 * implementations. Via InterfaceImpl the object are registered within
 * InterprocessServer class to be executed during communication.
 */
class InterfaceImpl {
 public:
  virtual ~InterfaceImpl() = default;

  /**
   * @brief GetInterface
   * @return request the mapping key during communication.
   */
  virtual interface_key GetInterface() const = 0;
};

/**
 * @brief The InterfaceListener class
 *
 * Interface for events listeners. No responce to client is expected.
 */
class InterfaceListener : public InterfaceImpl {
 public:
  virtual ~InterfaceListener() = default;

  /**
   * @brief HandleEvent
   * @param[in] function number from the inteface to execute.
   * @param[in] input parameters to be parsed and passed to function.
   * @return ResponceCodes indicating status.
   */
  virtual ResponseCodes HandleEvent(std::uint8_t function,
                                    const InputParameters& inputs) = 0;
};

/**
 * @brief The InterfaceExecutor class
 *
 * Interface for request executors.
 */
class InterfaceExecutor : public InterfaceListener {
 public:
  virtual ~InterfaceExecutor() = default;

  /**
   * @brief HandleRequest
   * @param[in] function number from the inteface to execute.
   * @param[in] input parameters to be parsed and passed to function.
   * @param[out] output to be send to client size.
   * @return ResponceCodes indicating status.
   */
  virtual ResponseCodes HandleRequest(std::uint8_t function,
                                      const InputParameters& inputs,
                                      OutputParameters& outputs) = 0;
};

}  // namespace InterprocessCommunication
}  // namespace Gengine
