#ifndef ELEMENT_MEDIATOR_H
#define ELEMENT_MEDIATOR_H

#include <memory>
#include <vector>

class ScrollBox;
class Element;
class Button;

/**
 * Mediator to mediate communication between buttons and the scrollbox in the item list
 * state. Enables buttons to change the sort method of the scrollbox.
 *
 * @see ScrollBox
 * @see Button
 * @see ItemList
 */
class Mediator : public std::enable_shared_from_this<Mediator> {
public:
  Mediator(const std::string& logFile);
  void addButton(std::shared_ptr<Button> button);
  void addScrollBox(std::shared_ptr<ScrollBox> scrollBox);
  void notify(std::shared_ptr<Element> sender, const std::string& event);

private:
  Logger logger;
  std::vector<std::shared_ptr<Button>> buttons;
  std::shared_ptr<ScrollBox> scrollBox;
};

#endif
