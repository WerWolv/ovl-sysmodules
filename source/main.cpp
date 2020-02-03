#include <tesla.hpp>    // The Tesla Header

class GuiMain : public tsl::Gui {
public:
    GuiMain() { }
    ~GuiMain() { }

    // Called when switching Guis to create the new UI
    virtual tsl::Element* createUI() {
        tsl::element::Frame *rootFrame = new tsl::element::Frame();
        
        return rootFrame;
    }

    // virtual void update() { }                        // Called once every frame to update values
    // virtual void onDestroy() { }                     // Called once when the Gui gets destroyed
    // virtual void preDraw(tsl::Screen *screen) { }    // Called before elements got drawn onto the screen
    // virtual void postDraw(tsl::Screen *screen) { }   // Called after elements got drawn onto the screen
};


class TemplateOverlay : public tsl::Overlay {
public:
    TemplateOverlay() { }
    ~TemplateOverlay() { }
    
    tsl::Gui* onSetup() { return new GuiMain(); }       // Called once when the Overlay is created and should return the first Gui to load. Initialize services here

    //virtual void onOverlayLoad(tsl::Gui *gui) { }     // Called once when the Overlay gets first loaded
    //virtual void onOverlayShow(tsl::Gui *gui) { }     // Called right before the overlay turns visible
    //virtual void onOverlayHide(tsl::Gui *gui) { }     // Called right before the overlay turns invisible
    //virtual void onDestroy() { }                      // Called once before the overlay Exits. Exit services here

};


// This function gets called on startup to create a new Overlay object
tsl::Overlay *overlayLoad() {
    return new TemplateOverlay();
}