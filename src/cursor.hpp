#ifndef NVUI_CURSOR_HPP
#define NVUI_CURSOR_HPP
#include <QRect>
#include <QTimer>
#include <cstdint>
#include <msgpack.hpp>
#include "hlstate.hpp"

enum class CursorShape : std::uint8_t
{
  Block,
  Horizontal,
  Vertical
};

enum class CursorStatus : std::uint8_t
{
  Visible,
  Hidden,
  Busy
};

/// Stores the cursor's position, in terms of a grid.
struct CursorPos
{
  std::uint16_t grid_num;
  int grid_x;
  int grid_y;
  int row;
  int col;
};

/// Describes the graphical properties of the cursor:
/// The area it occupies (in terms of a single cell), as well as
/// its highlight attribute, as well as whether the cell it contains
/// needs to be redrawn (with hl_id). This is true for Block shape.
struct CursorRect
{
  QRectF rect {0, 0, 0, 0};
  int hl_id = 0;
  bool should_draw_text = false;
};

/// Describes the information contained within a single mode of the cursor.
struct ModeInfo
{
  CursorShape cursor_shape = CursorShape::Block;
  int cell_percentage = 0;
  int blinkwait = 0;
  int blinkon = 0;
  int blinkoff = 0;
  int attr_id = 0;
  int attr_id_lm = 0;
  std::string short_name;
  std::string name;
  // (mouse_shape)
};

/// The Cursor class stores the data for the Neovim cursor
class Cursor : public QObject
{
  Q_OBJECT
public:
  Cursor();
  /**
   * Handles a 'mode_info_set' Neovim redraw event.
   */
  void mode_info_set(const msgpack::object* obj, std::uint32_t size);
  /**
   * Handles a 'mode_change' Neovim redraw event.
   */
  void mode_change(const msgpack::object* obj, std::uint32_t size);
  /**
   * Set the current position to pos, and refresh the cursor.
   */
  void go_to(CursorPos pos);
  /**
   * Get the rectangle the cursor occupies at the current moment.
   * This in pixels, and is calculated using the given font width and font height.
   */
  std::optional<CursorRect> rect(float font_width, float font_height) const noexcept;
  /**
   * Same as rect(), but based on the old cursor position and mode.
   */
  std::optional<CursorRect> old_rect(float font_width, float font_height) const noexcept;
  /**
   * Get the old position of the cursor (if it has an old position).
   */
  inline std::optional<CursorPos> old_pos() const noexcept
  {
    return prev_pos;
  }
  /**
   * Get the current position of the cursor (if it exists).
   */
  inline std::optional<CursorPos> pos() const noexcept
  {
    return cur_pos;
  }
  /**
   * Whether the cursor is hidden or not
   * If it's hidden, don't draw it
   */
  inline bool hidden() const noexcept
  {
    return status == CursorStatus::Hidden || busy();
  }
  /**
   * Hides the cursor, and disables all the timers so
   * that the cursor doesn't come back on again until
   * busy_stop
   */
  void busy_start();
  /**
   * Ends the Busy state, resetting the timers and
   * showing the cursor once again
   */
  void busy_stop();
private:
  int cell_width;
  int cell_height;
  CursorStatus status = CursorStatus::Visible;
  QTimer blinkwait_timer;
  QTimer blinkon_timer;
  QTimer blinkoff_timer;
  int row = 0;
  int col = 0;
  std::optional<CursorPos> cur_pos;
  std::optional<CursorPos> prev_pos;
  std::vector<ModeInfo> mode_info;
  ModeInfo cur_mode;
  std::size_t cur_mode_idx;
  std::size_t old_mode_idx;
  std::string cur_mode_name;
signals:
  void cursor_visible();
  void cursor_hidden();
private:
  /**
   * Stop/restart the timers.
   * This should be activated after the mode
   * has changed.
   */
  void reset_timers() noexcept;
  /**
   * Shows the cursor and sets the 'blinkon' timer,
   * which will start the 'blinkoff' timer when it
   * times out.
   */
  void set_blinkon_timer(int ms) noexcept;
  /**
   * Hides the cursor and starts the blinkoff
   * timer, which will start the 'blinkon' timer
   * when it times out.
   */
  void set_blinkoff_timer(int ms) noexcept;
  /**
   * Hide the cursor and set the status to Hidden.
   */
  void hide() noexcept;
  /**
   * Show the cursor and set the status to Visible.
   */
  void show() noexcept;
  /**
   * Whether the cursor is busy or not.
   */
  inline bool busy() const noexcept
  {
    return status == CursorStatus::Busy;
  }
};

#endif // NVUI_CURSOR_HPP