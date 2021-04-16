#define TEST_SCREEN 0
#define RESULT_SCREEN 1
#define START_SCREEN 2
#define HISTORY_SCREEN 3
#define PREF_SCREEN 4
#define CALIBRATION_SCREEN 5

class Screen {
  public:
    virtual void up_btn(GButton& up) {}
    virtual void down_btn(GButton& down) {}
    virtual void begin() {}
    virtual void _do(LCD& lcd) {}
    virtual uint8_t switch_to() { return 255;}
};
