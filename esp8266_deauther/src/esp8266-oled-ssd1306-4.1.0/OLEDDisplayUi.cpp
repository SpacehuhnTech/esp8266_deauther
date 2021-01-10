/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 by ThingPulse, Daniel Eichhorn
 * Copyright (c) 2018 by Fabrice Weinberg
 * Copyright (c) 2019 by Helmut Tschemernjak - www.radioshuttle.de
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ThingPulse invests considerable time and money to develop these open source libraries.
 * Please support us by buying our products (and not the clones) from
 * https://thingpulse.com
 *
 */

#include "OLEDDisplayUi.h"

void LoadingDrawDefault(OLEDDisplay *display, LoadingStage* stage, uint8_t progress) {
      display->setTextAlignment(TEXT_ALIGN_CENTER);
      display->setFont(ArialMT_Plain_10);
      display->drawString(64, 18, stage->process);
      display->drawProgressBar(4, 32, 120, 8, progress);
};


OLEDDisplayUi::OLEDDisplayUi(OLEDDisplay *display) {
  this->display = display;
	
  indicatorPosition = BOTTOM;
  indicatorDirection = LEFT_RIGHT;
  activeSymbol = ANIMATION_activeSymbol;
  inactiveSymbol = ANIMATION_inactiveSymbol;
  frameAnimationDirection   = SLIDE_RIGHT;
  lastTransitionDirection = 1;
  ticksPerFrame = 151; // ~ 5000ms at 30 FPS
  ticksPerTransition = 15;  // ~  500ms at 30 FPS
  frameCount = 0;
  nextFrameNumber = -1;
  overlayCount = 0;
  indicatorDrawState = 1;
  loadingDrawFunction = LoadingDrawDefault;
  updateInterval = 33;
  state.lastUpdate = 0;
  state.ticksSinceLastStateSwitch = 0;
  state.frameState = FIXED;
  state.currentFrame = 0;
  state.frameTransitionDirection = 1;
  state.isIndicatorDrawen = true;
  state.manuelControll = false;
  state.userData = NULL;
  shouldDrawIndicators = true;
  autoTransition = true;
}

void OLEDDisplayUi::init() {
  this->display->init();
}

void OLEDDisplayUi::setTargetFPS(uint8_t fps){
  float oldInterval = this->updateInterval;
  this->updateInterval = ((float) 1.0 / (float) fps) * 1000;

  // Calculate new ticksPerFrame
  float changeRatio = oldInterval / (float) this->updateInterval;
  this->ticksPerFrame *= changeRatio;
  this->ticksPerTransition *= changeRatio;
}

// -/------ Automatic controll ------\-

void OLEDDisplayUi::enableAutoTransition(){
  this->autoTransition = true;
}
void OLEDDisplayUi::disableAutoTransition(){
  this->autoTransition = false;
}
void OLEDDisplayUi::setAutoTransitionForwards(){
  this->state.frameTransitionDirection = 1;
  this->lastTransitionDirection = 1;
}
void OLEDDisplayUi::setAutoTransitionBackwards(){
  this->state.frameTransitionDirection = -1;
  this->lastTransitionDirection = -1;
}
void OLEDDisplayUi::setTimePerFrame(uint16_t time){
  this->ticksPerFrame = (uint16_t) ( (float) time / (float) updateInterval);
}
void OLEDDisplayUi::setTimePerTransition(uint16_t time){
  this->ticksPerTransition = (uint16_t) ( (float) time / (float) updateInterval);
}

// -/------ Customize indicator position and style -------\-
void OLEDDisplayUi::enableIndicator(){
  this->state.isIndicatorDrawen = true;
}

void OLEDDisplayUi::disableIndicator(){
  this->state.isIndicatorDrawen = false;
}

void OLEDDisplayUi::enableAllIndicators(){
  this->shouldDrawIndicators = true;
}

void OLEDDisplayUi::disableAllIndicators(){
  this->shouldDrawIndicators = false;
}

void OLEDDisplayUi::setIndicatorPosition(IndicatorPosition pos) {
  this->indicatorPosition = pos;
}
void OLEDDisplayUi::setIndicatorDirection(IndicatorDirection dir) {
  this->indicatorDirection = dir;
}
void OLEDDisplayUi::setActiveSymbol(const uint8_t* symbol) {
  this->activeSymbol = symbol;
}
void OLEDDisplayUi::setInactiveSymbol(const uint8_t* symbol) {
  this->inactiveSymbol = symbol;
}


// -/----- Frame settings -----\-
void OLEDDisplayUi::setFrameAnimation(AnimationDirection dir) {
  this->frameAnimationDirection = dir;
}
void OLEDDisplayUi::setFrames(FrameCallback* frameFunctions, uint8_t frameCount) {
  this->frameFunctions = frameFunctions;
  this->frameCount     = frameCount;
  this->resetState();
}

// -/----- Overlays ------\-
void OLEDDisplayUi::setOverlays(OverlayCallback* overlayFunctions, uint8_t overlayCount){
  this->overlayFunctions = overlayFunctions;
  this->overlayCount     = overlayCount;
}

// -/----- Loading Process -----\-

void OLEDDisplayUi::setLoadingDrawFunction(LoadingDrawFunction loadingDrawFunction) {
  this->loadingDrawFunction = loadingDrawFunction;
}

void OLEDDisplayUi::runLoadingProcess(LoadingStage* stages, uint8_t stagesCount) {
  uint8_t progress = 0;
  uint8_t increment = 100 / stagesCount;

  for (uint8_t i = 0; i < stagesCount; i++) {
    display->clear();
    this->loadingDrawFunction(this->display, &stages[i], progress);
    display->display();

    stages[i].callback();

    progress += increment;
    yield();
  }

  display->clear();
  this->loadingDrawFunction(this->display, &stages[stagesCount-1], progress);
  display->display();

  delay(150);
}

// -/----- Manuel control -----\-
void OLEDDisplayUi::nextFrame() {
  if (this->state.frameState != IN_TRANSITION) {
    this->state.manuelControll = true;
    this->state.frameState = IN_TRANSITION;
    this->state.ticksSinceLastStateSwitch = 0;
    this->lastTransitionDirection = this->state.frameTransitionDirection;
    this->state.frameTransitionDirection = 1;
  }
}
void OLEDDisplayUi::previousFrame() {
  if (this->state.frameState != IN_TRANSITION) {
    this->state.manuelControll = true;
    this->state.frameState = IN_TRANSITION;
    this->state.ticksSinceLastStateSwitch = 0;
    this->lastTransitionDirection = this->state.frameTransitionDirection;
    this->state.frameTransitionDirection = -1;
  }
}

void OLEDDisplayUi::switchToFrame(uint8_t frame) {
  if (frame >= this->frameCount) return;
  this->state.ticksSinceLastStateSwitch = 0;
  if (frame == this->state.currentFrame) return;
  this->state.frameState = FIXED;
  this->state.currentFrame = frame;
  this->state.isIndicatorDrawen = true;
}

void OLEDDisplayUi::transitionToFrame(uint8_t frame) {
  if (frame >= this->frameCount) return;
  this->state.ticksSinceLastStateSwitch = 0;
  if (frame == this->state.currentFrame) return;
  this->nextFrameNumber = frame;
  this->lastTransitionDirection = this->state.frameTransitionDirection;
  this->state.manuelControll = true;
  this->state.frameState = IN_TRANSITION;
  this->state.frameTransitionDirection = frame < this->state.currentFrame ? -1 : 1;
}


// -/----- State information -----\-
OLEDDisplayUiState* OLEDDisplayUi::getUiState(){
  return &this->state;
}

int16_t OLEDDisplayUi::update(){
#ifdef ARDUINO
  unsigned long frameStart = millis();
#elif __MBED__
	Timer t;
	t.start();
	unsigned long frameStart = t.read_ms();
#else
#error "Unkown operating system"
#endif
  int16_t timeBudget = this->updateInterval - (frameStart - this->state.lastUpdate);
  if ( timeBudget <= 0) {
    // Implement frame skipping to ensure time budget is keept
    if (this->autoTransition && this->state.lastUpdate != 0) this->state.ticksSinceLastStateSwitch += ceil((double)-timeBudget / (double)this->updateInterval);

    this->state.lastUpdate = frameStart;
    this->tick();
  }
#ifdef ARDUINO
  return this->updateInterval - (millis() - frameStart);
#elif __MBED__
  return this->updateInterval - (t.read_ms() - frameStart);
#else
#error "Unkown operating system"
#endif
}


void OLEDDisplayUi::tick() {
  this->state.ticksSinceLastStateSwitch++;

  switch (this->state.frameState) {
    case IN_TRANSITION:
        if (this->state.ticksSinceLastStateSwitch >= this->ticksPerTransition){
          this->state.frameState = FIXED;
          this->state.currentFrame = getNextFrameNumber();
          this->state.ticksSinceLastStateSwitch = 0;
          this->nextFrameNumber = -1;
        }
      break;
    case FIXED:
      // Revert manuelControll
      if (this->state.manuelControll) {
        this->state.frameTransitionDirection = this->lastTransitionDirection;
        this->state.manuelControll = false;
      }
      if (this->state.ticksSinceLastStateSwitch >= this->ticksPerFrame){
          if (this->autoTransition){
            this->state.frameState = IN_TRANSITION;
          }
          this->state.ticksSinceLastStateSwitch = 0;
      }
      break;
  }

  this->display->clear();
  this->drawFrame();
  if (shouldDrawIndicators) {
    this->drawIndicator();
  }
  this->drawOverlays();
  this->display->display();
}

void OLEDDisplayUi::resetState() {
  this->state.lastUpdate = 0;
  this->state.ticksSinceLastStateSwitch = 0;
  this->state.frameState = FIXED;
  this->state.currentFrame = 0;
  this->state.isIndicatorDrawen = true;
}

void OLEDDisplayUi::drawFrame(){
  switch (this->state.frameState){
     case IN_TRANSITION: {
       float progress = (float) this->state.ticksSinceLastStateSwitch / (float) this->ticksPerTransition;
       int16_t x = 0, y = 0, x1 = 0, y1 = 0;
       switch(this->frameAnimationDirection){
        case SLIDE_LEFT:
          x = -this->display->width() * progress;
          y = 0;
          x1 = x + this->display->width();
          y1 = 0;
          break;
        case SLIDE_RIGHT:
          x = this->display->width() * progress;
          y = 0;
          x1 = x - this->display->width();
          y1 = 0;
          break;
        case SLIDE_UP:
          x = 0;
          y = -this->display->height() * progress;
          x1 = 0;
          y1 = y + this->display->height();
          break;
        case SLIDE_DOWN:
        default:
          x = 0;
          y = this->display->height() * progress;
          x1 = 0;
          y1 = y - this->display->height();
          break;
       }

       // Invert animation if direction is reversed.
       int8_t dir = this->state.frameTransitionDirection >= 0 ? 1 : -1;
       x *= dir; y *= dir; x1 *= dir; y1 *= dir;

       bool drawenCurrentFrame;


       // Prope each frameFunction for the indicator Drawen state
       this->enableIndicator();
       (this->frameFunctions[this->state.currentFrame])(this->display, &this->state, x, y);
       drawenCurrentFrame = this->state.isIndicatorDrawen;

       this->enableIndicator();
       (this->frameFunctions[this->getNextFrameNumber()])(this->display, &this->state, x1, y1);

       // Build up the indicatorDrawState
       if (drawenCurrentFrame && !this->state.isIndicatorDrawen) {
         // Drawen now but not next
         this->indicatorDrawState = 2;
       } else if (!drawenCurrentFrame && this->state.isIndicatorDrawen) {
         // Not drawen now but next
         this->indicatorDrawState = 1;
       } else if (!drawenCurrentFrame && !this->state.isIndicatorDrawen) {
         // Not drawen in both frames
         this->indicatorDrawState = 3;
       }

       // If the indicator isn't draw in the current frame
       // reflect it in state.isIndicatorDrawen
       if (!drawenCurrentFrame) this->state.isIndicatorDrawen = false;

       break;
     }
     case FIXED:
      // Always assume that the indicator is drawn!
      // And set indicatorDrawState to "not known yet"
      this->indicatorDrawState = 0;
      this->enableIndicator();
      (this->frameFunctions[this->state.currentFrame])(this->display, &this->state, 0, 0);
      break;
  }
}

void OLEDDisplayUi::drawIndicator() {

    // Only draw if the indicator is invisible
    // for both frames or
    // the indiactor is shown and we are IN_TRANSITION
    if (this->indicatorDrawState == 3 || (!this->state.isIndicatorDrawen && this->state.frameState != IN_TRANSITION)) {
      return;
    }

    uint8_t posOfHighlightFrame = 0;
    float indicatorFadeProgress = 0;

    // if the indicator needs to be slided in we want to
    // highlight the next frame in the transition
    uint8_t frameToHighlight = this->indicatorDrawState == 1 ? this->getNextFrameNumber() : this->state.currentFrame;

    // Calculate the frame that needs to be highlighted
    // based on the Direction the indiactor is drawn
    switch (this->indicatorDirection){
      case LEFT_RIGHT:
        posOfHighlightFrame = frameToHighlight;
        break;
      case RIGHT_LEFT:
      default:
        posOfHighlightFrame = this->frameCount - frameToHighlight;
        break;
    }

    switch (this->indicatorDrawState) {
      case 1: // Indicator was not drawn in this frame but will be in next
        // Slide IN
        indicatorFadeProgress = 1 - ((float) this->state.ticksSinceLastStateSwitch / (float) this->ticksPerTransition);
        break;
      case 2: // Indicator was drawn in this frame but not in next
        // Slide OUT
        indicatorFadeProgress = ((float) this->state.ticksSinceLastStateSwitch / (float) this->ticksPerTransition);
        break;
    }

    //Space between indicators - reduce for small screen sizes
    uint16_t indicatorSpacing = 12;
    if (this->display->getHeight() < 64 && (this->indicatorPosition == RIGHT || this->indicatorPosition == LEFT)) {
      indicatorSpacing = 6;
    }

    uint16_t frameStartPos = (indicatorSpacing * frameCount / 2);
    const uint8_t *image;

    uint16_t x = 0,y = 0;


    for (uint8_t i = 0; i < this->frameCount; i++) {

      switch (this->indicatorPosition){
        case TOP:
          y = 0 - (8 * indicatorFadeProgress);
          x = (this->display->width() / 2) - frameStartPos + 12 * i;
          break;
        case BOTTOM:
          y = (this->display->height() - 8) + (8 * indicatorFadeProgress);
          x = (this->display->width() / 2) - frameStartPos + 12 * i;
          break;
        case RIGHT:
          x = (this->display->width() - 8) + (8 * indicatorFadeProgress);
          y = (this->display->height() / 2) - frameStartPos + 2 + 12 * i;
          break;
        case LEFT:
        default:
          x = 0 - (8 * indicatorFadeProgress);
          y = (this->display->height() / 2) - frameStartPos + 2 + indicatorSpacing * i;
          break;
      }

      if (posOfHighlightFrame == i) {
         image = this->activeSymbol;
      } else {
         image = this->inactiveSymbol;
      }

      this->display->drawFastImage(x, y, 8, 8, image);
    }
}

void OLEDDisplayUi::drawOverlays() {
 for (uint8_t i=0;i<this->overlayCount;i++){
    (this->overlayFunctions[i])(this->display, &this->state);
 }
}

uint8_t OLEDDisplayUi::getNextFrameNumber(){
  if (this->nextFrameNumber != -1) return this->nextFrameNumber;
  return (this->state.currentFrame + this->frameCount + this->state.frameTransitionDirection) % this->frameCount;
}
