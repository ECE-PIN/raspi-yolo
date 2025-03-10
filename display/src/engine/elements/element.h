#ifndef ELEMENT_H
#define ELEMENT_H

#include <SDL2/SDL.h>
#include <memory>

#include "../../../../logger.h"
#include "../display_global.h"
#include "element_mediator.h"

/**
 * Defines the interface for an SDL element. This is any basic shape or texture to be
 * rendered to the screen.
 */
class Element : public std::enable_shared_from_this<Element> {
public:
  virtual ~Element() = default;
  virtual void addElement(std::shared_ptr<Element> element) {}
  virtual void update();
  virtual void render() const;
  virtual void handleEvent(const SDL_Event& event) = 0;

  virtual std::string getContent() const;
  virtual void setContent(const std::string& content) {}

  void setMediator(std::shared_ptr<Mediator> mediator);
  void setParent(Element* parent);

  SDL_Point getPositionRelativeToParent();
  void setPositionRelativeToParent(const SDL_Point& relativePosition);

  SDL_Rect getBoundaryRectangle();
  void setBoundaryRectangle(SDL_Rect boundaryRectangle);

  void setCentered();

  void setCenteredVertical();
  bool checkCenterVertical();
  void centerVertical();

  void setCenteredHorizontal();
  bool checkCenterHorizontal();
  void centerHorizontal();

  bool checkHovered();

  void addBorder(const int& borderThickness);
  void renderBorder() const;

protected:
  std::unique_ptr<Logger> logger;
  std::string logFile;

  struct DisplayGlobal displayGlobal;
  int id                             = -1;
  SDL_Rect boundaryRectangle         = {0, 0, 0, 0};
  SDL_Point positionRelativeToParent = {0, 0};
  Element* parent                    = nullptr;
  bool hasBorder                     = false;
  int borderThickness                = 1;
  bool centerWithinParent            = false;
  bool centerVerticalWithinParent    = false;
  bool centerHorizontalWithinParent  = false;

  std::weak_ptr<Mediator> mediator;

  void setupPosition(const SDL_Rect& boundaryRectangle);
  void hasParentUpdate();
  void updatePosition();
};

#endif
