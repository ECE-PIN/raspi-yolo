#include <glog/logging.h>

#include "button.h"
#include "dropdown.h"
#include "element.h"
#include "element_mediator.h"
#include "scroll_box.h"
#include "sort_method.h"

Mediator::Mediator(const std::string& logFile) : logger(logFile) {}

void Mediator::addButton(std::shared_ptr<Button> button) {
  this->buttons.push_back(button);
  button->setMediator(shared_from_this());
}

void Mediator::addScrollBox(std::shared_ptr<ScrollBox> scrollBox) {
  this->scrollBox = scrollBox;
  scrollBox->setMediator(shared_from_this());
}

/**
 * Alert the scrollbox that a new sort method is to be used.
 *
 * @param sender Which element sent the message to the mediator (currently unused)
 * @param event The message sent to the mediator
 * @return None
 */
void Mediator::notify(std::shared_ptr<Element> sender, const std::string& event) {
  if (event == "low to high") {
    this->logger.log("Mediator received low to high notification");
    this->scrollBox->setSortMethod(SortMethod::LOW_TO_HIGH);
  }
  else if (event == "high to low") {
    this->logger.log("Mediator received high to low notification");
    this->scrollBox->setSortMethod(SortMethod::HIGH_TO_LOW);
  }
  else {
    LOG(FATAL) << "Mediator received invalid notification";
  }
}
