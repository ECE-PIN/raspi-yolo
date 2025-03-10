#ifndef COMPOSITE_ELEMENT_H
#define COMPOSITE_ELEMENT_H

#include <vector>

#include "element.h"

/**
 * An element that contains other elements within it.
 */
class CompositeElement : public Element {
public:
  void addElement(std::shared_ptr<Element> element) override;
  void update() override;
  void render() const override;
  void handleEvent(const SDL_Event& event) override;

protected:
  virtual void updateSelf();
  virtual void renderSelf() const;
  virtual void handleEventSelf(const SDL_Event& event) = 0;

  std::vector<std::shared_ptr<Element>> children;
};

#endif
