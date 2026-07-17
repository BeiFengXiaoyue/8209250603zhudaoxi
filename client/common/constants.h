#ifndef CONSTANTS_H
#define CONSTANTS_H

// ============================================================
// 布局常量（Layout Constants）
// ============================================================
constexpr int kPanelWidth        = 260;   // 左侧信息面板宽度
constexpr int kSidebarWidth      = 210;   // 右侧导航栏宽度
constexpr int kAvatarSize        = 100;   // 大号头像尺寸
constexpr int kSmallAvatarSize   = 36;    // 小号头像尺寸
constexpr int kReplyAvatarSize   = 28;    // 回复头像尺寸
constexpr int kHeaderAvatarSize  = 42;    // 页面头部头像尺寸
constexpr int kNavButtonHeight   = 48;    // 导航栏按钮高度
constexpr int kEditButtonHeight  = 38;    // 编辑资料按钮高度
constexpr int kMiniButtonSize    = 36;    // 小按钮尺寸（前后翻页）
constexpr int kTabButtonHeight   = 40;    // Tab 按钮高度
constexpr int kBottomNavHeight   = 50;    // 底部导航栏高度
constexpr int kSearchInputHeight = 50;    // 搜索输入框高度
constexpr int kFilterChipHeight  = 30;    // 筛选标签高度
constexpr int kTagButtonHeight   = 32;    // 标签按钮高度
constexpr int kLineButtonHeight  = 30;    // 行内按钮高度（点赞/回复）
constexpr int kExpandBtnHeight   = 34;    // 展开按钮高度
constexpr int kHeaderRowHeight   = 44;    // 顶部行高
constexpr int kTabBarHeight      = 52;    // Tab 栏高度

// ============================================================
// 卡片尺寸（Card Dimensions）
// ============================================================
constexpr int kCardWidth         = 190;   // 卡片宽度
constexpr int kCardHeight        = 150;   // 卡片高度
constexpr int kCardThumbHeight   = 90;    // 卡片缩略图高度
constexpr int kCardTitleHeight   = 18;    // 卡片标题高度
constexpr int kCardSubtitleHeight = 16;   // 卡片副标题高度

// ============================================================
// 布局间距（Layout Spacing & Margins）
// ============================================================
constexpr int kMarginL          = 20;    // 大边距
constexpr int kMarginM          = 16;    // 中间距
constexpr int kMarginS          = 12;    // 小边距
constexpr int kMarginXS         = 8;     // 极小边距
constexpr int kMarginXXS        = 4;     // 最小区距
constexpr int kMarginNone       = 0;     // 无边距
constexpr int kSpacingL         = 20;    // 大间距
constexpr int kSpacingM         = 15;    // 中间距
constexpr int kSpacingS         = 12;    // 小间距
constexpr int kSpacingXS        = 8;     // 极小间距
constexpr int kSpacingXXS       = 4;     // 最小区距
constexpr int kSpacingTab       = 2;     // Tab 间距
constexpr int kSpacingGrid      = 15;    // 网格间距

// 面板内边距
constexpr int kPanelPadTop      = 30;    // 面板上边距
constexpr int kPanelPadBottom   = 25;    // 面板下边距
constexpr int kPanelPadLeft     = 20;    // 面板左边距
constexpr int kPanelPadRight    = 20;    // 面板右边距
constexpr int kSidebarPadTop    = 20;    // 侧栏上边距
constexpr int kSidebarPadBottom = 20;    // 侧栏下边距
constexpr int kSidebarPadLeft   = 12;    // 侧栏左边距
constexpr int kSidebarPadRight  = 12;    // 侧栏右边距

// ============================================================
// 动画常量（Animation Constants）
// ============================================================
constexpr int kAnimNormal     = 350;   // 标准动画时长（ms）
constexpr int kAnimFast       = 300;   // 快速动画时长（ms）
constexpr int kAnimSlow       = 400;   // 慢速动画时长（ms）

// ============================================================
// 阴影常量（Shadow Constants）
// ============================================================
constexpr int kShadowSmall    = 12;    // 小阴影模糊半径
constexpr int kShadowNormal   = 15;    // 标准阴影模糊半径
constexpr int kShadowLarge    = 20;    // 大阴影模糊半径
constexpr int kShadowOffsetY  = 2;     // 阴影 Y 偏移

// ============================================================
// 网格布局（Grid Layout）
// ============================================================
constexpr int kGridCols       = 3;     // 标准网格列数
constexpr int kVideoGridCols  = 4;     // 视频卡片网格列数
constexpr int kCardsPerPage   = 6;     // 每页卡片数（3×2）
constexpr int kCardsPerRow    = 3;     // 每行卡片数

// ============================================================
// 分页常量（Pagination）
// ============================================================
constexpr int kTabCount       = 4;     // 内容区 Tab 数量
constexpr int kMaxPageItems   = 6;     // 每页最大条目数

// ============================================================
// 最小窗口尺寸（Minimum Window Size）
// ============================================================
constexpr int kMinWindowWidth  = 1200;  // 窗口最小宽度
constexpr int kMinWindowHeight = 720;   // 窗口最小高度

// ============================================================
// 圆角半径（Border Radius）
// ============================================================
constexpr int kRadiusLarge    = 15;    // 大圆角
constexpr int kRadiusMedium   = 12;    // 中圆角
constexpr int kRadiusSmall    = 10;    // 小圆角
constexpr int kRadiusXSmall   = 8;     // 极小圆角
constexpr int kRadiusRound    = 18;    // 全圆按钮

// ============================================================
// 字体大小（Font Sizes）
// ============================================================
constexpr int kFontXL         = 24;    // 超大字体
constexpr int kFontL          = 18;    // 大字体
constexpr int kFontM          = 16;    // 中字体
constexpr int kFontBody       = 14;    // 正文
constexpr int kFontS          = 13;    // 小字体
constexpr int kFontXS         = 12;    // 极小字体
constexpr int kFontXXS        = 11;    // 超小字体

// ============================================================
// 色值参考（Named Colors - 仅用于 QColor 构造）
// ============================================================
// 主色调: #5B7DB1 / #3B5998
// 文本色: #2C3E50 / #555555 / #888888 / #999999 / #AAAAAA / #CCCCCC
// 背景色: #F5F7FA / #FFFFFF
// 边框色: #E0E4E8 / #ECF0F1 / #D0D5DD
// 强调色: #E74C3C / #4A7C59 / #F5A623

#endif // CONSTANTS_H
