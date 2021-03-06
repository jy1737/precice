#ifndef PRECICE_IO_EXPORTCONFIGURATION_HPP_
#define PRECICE_IO_EXPORTCONFIGURATION_HPP_

#include "io/ExportContext.hpp"
#include "io/Constants.hpp"
#include "io/SharedPointer.hpp"
#include "xml/XMLTag.hpp"
#include "logging/Logger.hpp"
#include "precice/Constants.hpp"
#include <string>
#include <list>

namespace precice {
namespace io {

/**
 * @brief Configuration class for exports.
 */
class ExportConfiguration : public xml::XMLTag::Listener
{
public:

  /**
   * @brief Name of the xml-tag corresponding to the ExportConfiguration.
   */
  //static const std::string& getTag();

  /**
   * @brief Constructor.
   */
  ExportConfiguration ( xml::XMLTag& parent );

  /**
   * @brief Parses the export configuration xml-tag.
   *
   * Requirements:
   * - xmlReader has to point to the tag corresponding to ExportConfiguration
   */
  //bool parseSubtag ( xml::XMLTag::XMLReader* xmlReader );

  /**
   * @brief Returns true, if xml-tag has been parsed successfully.
   */
  //bool isValid() const { return _isValid; }

  /**
   * @brief Returns the configured export context, valid if isValid() is true.
   */
  std::list<ExportContext>& exportContexts() { return _contexts; }

  /**
   * @brief Callback function required for use of automatic configuration.
   *
   * Is called by xml::XMLTag on automatic configuration every time an xml
   * tag and its attributes have been read.
   * @param callingTag [IN] XML tag currently read.
   * @param xmlReader  [IN] XML Reader responsible for reading the tag.
   * @return True, if the corresponding actions could be successfully performed.
   */
  virtual void xmlTagCallback ( xml::XMLTag& callingTag );

  /**
   * @brief Callback from automatic configuration. Not utilitzed here.
   */
  virtual void xmlEndTagCallback ( xml::XMLTag& callingTag ) {}

  void resetExports() { _contexts.clear(); }

private:
  logging::Logger _log{"io::ExportConfiguration"};

  const std::string TAG;

  const std::string ATTR_LOCATION;
  const std::string ATTR_TYPE;
  const std::string ATTR_AUTO;
  const std::string VALUE_VTK;

  const std::string ATTR_TIMESTEP_INTERVAL;
  const std::string ATTR_NEIGHBORS;
  const std::string ATTR_TRIGGER_SOLVER;
  const std::string ATTR_NORMALS;
  const std::string ATTR_EVERY_ITERATION;

  std::list<ExportContext> _contexts;
};

}} // namespace precice, io

#endif /* PRECICE_IO_EXPORTCONFIGURATION_HPP_ */
