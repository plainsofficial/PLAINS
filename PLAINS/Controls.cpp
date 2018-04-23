#include <iostream>

#include "Controls.h"
#include "DrawManager.h"
#include "Menu.h"

#define white Color(255, 255, 255)
#define mixed Color(90, 90, 90)
#define lighter_gray Color(48, 48, 48)
#define light_gray Color(40, 40, 40)
#define gray Color(28, 28, 28)
#define dark_gray Color(21, 21, 19)
#define darker_gray Color(19, 19, 19)
#define black Color(0, 0, 0)

#pragma region Base Control
void CControl::SetPosition(int x, int y)
{
	m_x = x;
	m_y = y;
}

void CControl::SetSize(int w, int h)
{
	m_iWidth = w;
	m_iHeight = h;
}

void CControl::GetSize(int &w, int &h)
{
	w = m_iWidth;
	h = m_iHeight;
}

bool CControl::Flag(int f)
{
	if (m_Flags & f)
		return true;
	else
		return false;
}

POINT CControl::GetAbsolutePos()
{
	POINT p;
	RECT client = parent->GetClientArea();
	if (parent)
	{
		p.x = m_x + client.left;
		p.y = m_y + client.top + 29;
	}
	
	return p;
}

void CControl::SetFileId(std::string fid)
{
	FileIdentifier = fid;
}
#pragma endregion Implementations of the Base control functions

#pragma region CheckBox
CCheckBox::CCheckBox()
{
	Checked = false;
	bIsSub = false;

	m_Flags = UIFlags::UI_Clickable | UIFlags::UI_Drawable | UIFlags::UI_SaveFile;
	m_iHeight = 9;

	FileControlType = UIControlTypes::UIC_CheckBox;
}

void CCheckBox::SetState(bool s)
{
	Checked = s;
}

bool CCheckBox::GetState()
{
	return Checked;
}

bool CCheckBox::GetIsSub()
{
	return bIsSub;
}

void CCheckBox::SetAsSub(bool t)
{
	bIsSub = t;
}

void CCheckBox::Draw(bool hover)
{
	POINT a = GetAbsolutePos();
	draw.gradient_verticle(a.x, a.y + 1, 18, 8, Color(62, 62, 62, 255), Color(55, 55, 55, 245));
	draw.outline(a.x, a.y + 1, 18, 8, Color(2, 2, 2, 255));

	Color grad;
	bool bSetRed = false;
	bool bSetGreen = false;
	bool bSetBlue = false;
	if (game::globals.forecolor.r() >= 15)
		bSetRed = true;
	if (game::globals.forecolor.g() >= 15)
		bSetGreen = true;
	if (game::globals.forecolor.b() >= 15)
		bSetBlue = true;

	float red = bSetRed ? game::globals.forecolor.r() - 15 : game::globals.forecolor.r();
	float green = bSetGreen ? game::globals.forecolor.g() - 15 : game::globals.forecolor.g();
	float blue = bSetBlue ? game::globals.forecolor.b() - 15 : game::globals.forecolor.b();

	grad = Color(red, green, blue, 245);

	Color unchecked = Color(miscconfig.cCheckboxUnchecked[0], miscconfig.cCheckboxUnchecked[1], miscconfig.cCheckboxUnchecked[2]);

	Color unchecked_grad;
	bool unchecked_bSetRed = false;
	bool unchecked_bSetGreen = false;
	bool unchecked_bSetBlue = false;
	if (unchecked.r() >= 15)
		unchecked_bSetRed = true;
	if (unchecked.g() >= 15)
		unchecked_bSetGreen = true;
	if (unchecked.b() >= 15)
		unchecked_bSetBlue = true;

	float unchecked_red = unchecked_bSetRed ? unchecked.r() - 15 : unchecked.r();
	float unchecked_green = unchecked_bSetGreen ? unchecked.g() - 15 : unchecked.g();
	float unchecked_blue = unchecked_bSetBlue ? unchecked.b() - 15 : unchecked.b();

	unchecked_grad = Color(unchecked_red, unchecked_green, unchecked_blue, 245);

	if (Checked != last_state) {
		should_animate = true;
	}

	if (should_animate) {
		if (Checked) {
			animation_state += 1;

			if (animation_state == 9) {
				should_animate = false;
				last_state = Checked;
			}
		}
		else {
			animation_state -= 1;

			if (animation_state == 0) {
				should_animate = false;
				last_state = Checked;
			}
		}
	}

	draw.gradient_verticle(a.x + animation_state, a.y + 1, 9, 8, Checked ? game::globals.forecolor : unchecked, Checked ? grad : unchecked_grad);
	draw.outline(a.x, a.y + 1, 18, 8, Color(2, 2, 2, 255));
}

void CCheckBox::OnUpdate(){ m_iHeight = 9; }

void CCheckBox::OnClick()
{
	if (!should_animate)
		Checked = !Checked;
}
#pragma endregion Implementations of the Check Box functions

#pragma region Label
CLabel::CLabel()
{
	m_Flags = UIFlags::UI_Drawable;
	FileControlType = UIC_Label;
	Text = "Default";
	FileIdentifier = "Default";
	m_iHeight = 10;
}

void CLabel::Draw(bool hover)
{
	POINT a = GetAbsolutePos();
	draw.text(a.x, a.y - 1, Text.c_str(), draw.fonts.menu, Color(205, 205, 205, 255));
}

void CLabel::SetText(std::string text)
{
	Text = text;
}

void CLabel::OnUpdate(){}
void CLabel::OnClick(){}
#pragma endregion Implementations of the Label functions

#pragma region GroupBox
CGroupBox::CGroupBox()
{
	Items = 1;
	last_y = 0;
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_RenderFirst;
	Text = "Default";
	FileIdentifier = "Default";
	FileControlType = UIControlTypes::UIC_GroupBox;
}
void CGroupBox::Draw(bool hover)
{
	POINT a = GetAbsolutePos();
	RECT txtSize = draw.get_text_size(Text.c_str(), draw.fonts.menu);
	draw.rect(a.x + 2, a.y + 2, m_iWidth - 4, m_iHeight - 4, Color(90, 90, 90, 1));
	draw.text(a.x + (m_iWidth / 2) - (txtSize.right / 2), a.y - (txtSize.bottom / 2) - 1, Text.c_str(), draw.fonts.menu, Color(244, 244, 244, 255));
	if (group_tabs.size())
	{
		draw.line(a.x + 1, a.y + 8, a.x + m_iWidth, a.y + 8, Color(48, 48, 48, 255));
		draw.line(a.x + 1, a.y + 38, a.x + m_iWidth, a.y + 38, Color(48, 48, 48, 255));
		draw.rect(a.x + 1, a.y + 9, m_iWidth - 1, 29, Color(21, 21, 19, 255));
		draw.rect(a.x + 1, a.y + 36, m_iWidth - 1, 2, Color(62, 62, 62, 255));

		for (int i = 0; i < group_tabs.size(); i++)
		{
			RECT text_size = draw.get_text_size(group_tabs[i].name.c_str(), draw.fonts.menu);

			int width = m_iWidth - 1;

			int tab_length = (width / group_tabs.size());

			int text_position[] = {
				(a.x + (tab_length * (i + 1)) - (tab_length / 2)),
				a.y + 23 - (text_size.bottom / 2)
			};

			RECT tab_area = {
				(a.x + 1) + (tab_length * i),
				a.y + 9,
				tab_length,
				29
			};

			if (GetAsyncKeyState(VK_LBUTTON))
			{
				if (GUI.IsMouseInRegion(tab_area))
				{
					selected_tab = group_tabs[i].id;
				}
			}

			if (selected_tab == group_tabs[i].id)
			{
				draw.rect(tab_area.left, tab_area.top, tab_area.right, tab_area.bottom, game::globals.forecolor);
				draw.rect(tab_area.left, tab_area.top, tab_area.right, tab_area.bottom - 2, Color(39, 39, 39, 255));
				draw.text(text_position[0] - (text_size.right / 2), text_position[1], group_tabs[i].name.c_str(), draw.fonts.menu, Color(205, 205, 205, 245));
			}
			else if (selected_tab != group_tabs[i].id)
				draw.text(text_position[0] - (text_size.right / 2), text_position[1], group_tabs[i].name.c_str(), draw.fonts.menu, Color(92, 92, 92, 255));
		}
	}

	//Top Left
	draw.line(a.x, a.y, a.x + (m_iWidth / 2) - (txtSize.right / 2) - 2, a.y, Color(48, 48, 48, 255));
	draw.line(a.x - 1, a.y - 1, a.x + (m_iWidth / 2) - (txtSize.right / 2) - 1, a.y - 1, Color(0, 0, 0, 255));

	//Top Right
	draw.line(a.x + (m_iWidth / 2) + (txtSize.right / 2) + 2, a.y, a.x + m_iWidth, a.y, Color(48, 48, 48, 255));
	draw.line(a.x + (m_iWidth / 2) + (txtSize.right / 2) + 2, a.y - 1, a.x + m_iWidth + 1, a.y - 1, Color(0, 0, 0, 255));

	//Left
	draw.line(a.x, a.y, a.x, a.y + m_iHeight, Color(49, 49, 49, 255));
	draw.line(a.x - 1, a.y, a.x - 1, a.y + m_iHeight, Color(0, 0, 0, 255));

	//Bottom
	draw.line(a.x, a.y + m_iHeight, a.x + m_iWidth, a.y + m_iHeight, Color(48, 48, 48, 255));
	draw.line(a.x - 1, a.y + m_iHeight + 1, a.x + m_iWidth + 2, a.y + m_iHeight + 1, Color(0, 0, 0, 255));

	//Right
	draw.line(a.x + m_iWidth, a.y, a.x + m_iWidth, a.y + m_iHeight + 1, Color(48, 48, 48, 255));
	draw.line(a.x + m_iWidth + 1, a.y, a.x + m_iWidth + 1, a.y + m_iHeight + 1, Color(0, 0, 0, 255));
}

void CGroupBox::SetText(std::string text)
{
	Text = text;
}

void CGroupBox::PlaceLabeledControl(int g_tab, std::string Label, CTab *Tab, CControl* control) {
	bool has_tabs = group_tabs.size() ? 1 : 0;

	if (has_tabs) {
		bool has_reset = false;

		for (int i = 0; i < reset_tabs.size(); i++) {
			if (reset_tabs[i] == g_tab)
				has_reset = true;
		}

		if (!has_reset) {
			initialized = false;
			reset_tabs.push_back(g_tab);
		}
	}

	if (!initialized) {
		Items = 0;
		last_y = has_tabs ? m_y + 48 : m_y + 8;
		initialized = true;
	}

	bool add_label_y = true;
	bool is_checkbox = control->FileControlType == UIControlTypes::UIC_CheckBox;
	bool is_label = control->FileControlType == UIControlTypes::UIC_Label;
	bool is_keybind = control->FileControlType == UIControlTypes::UIC_KeyBind;
	bool is_color = control->FileControlType == UIControlTypes::UIC_ColorSelector;

	int x = m_x + 38;
	int y = last_y;
	int control_width, control_height;
	control->GetSize(control_width, control_height);

	CLabel* label = new CLabel;
	label->SetPosition(x, y);
	label->SetText(Label);
	label->parent_group = this;
	label->g_tab = g_tab ? g_tab : 0;
	Tab->RegisterControl(label);

	if (is_checkbox || is_label || is_color) add_label_y = false;

	if (Label != "" && add_label_y && !is_keybind) {
		RECT label_size = draw.get_text_size(Label.c_str(), draw.fonts.menu);
		last_y += 14;
		y = last_y;
	}

	if (!is_keybind)
		last_control_height = control_height + 7;

	if (is_keybind || (is_color && Label == "")) {
		y -= last_control_height;
		x = m_x + m_iWidth - 36;
	}
	if (is_color && Label != "")
		x = m_x + m_iWidth - 36;
	if (is_checkbox)
		x -= 24;

	control->SetPosition(x, is_checkbox ? y + 1 : y);
	control->SetSize(m_iWidth - (38 * 2), control_height);
	control->parent_group = this;
	control->g_tab = g_tab ? g_tab : 0;
	Tab->RegisterControl(control);

	if (!is_keybind) {
		if (!is_color || is_color && Label != "")
			last_y += control_height + 7;
	}
}

void CGroupBox::AddTab(CGroupTab t)
{
	group_tabs.push_back(t);

	if (selected_tab == 0)
		selected_tab++;
}
void CGroupBox::OnUpdate(){}
void CGroupBox::OnClick(){}
#pragma endregion Implementations of the Group Box functions

#pragma region Sliders
CSlider::CSlider()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_SaveFile;
	Format = FORMAT_INT;
	m_iHeight = 10;
	FileControlType = UIControlTypes::UIC_Slider;
}

void CSlider::Draw(bool hover)
{
	POINT a = GetAbsolutePos();

	draw.gradient_verticle(a.x, a.y, m_iWidth, 7, Color(62, 62, 62, 255), Color(55, 55, 55, 245));

	float Ratio = ( Value - Min ) / ( Max - Min );
	float Location = Ratio*m_iWidth;

	Color grad;
	bool bSetRed = false;
	bool bSetGreen = false;
	bool bSetBlue = false;
	if (game::globals.forecolor.r() >= 15)
		bSetRed = true;
	if (game::globals.forecolor.g() >= 15)
		bSetGreen = true;
	if (game::globals.forecolor.b() >= 15)
		bSetBlue = true;

	float red = bSetRed ? game::globals.forecolor.r() - 15 : game::globals.forecolor.r();
	float green = bSetGreen ? game::globals.forecolor.g() - 15 : game::globals.forecolor.g();
	float blue = bSetBlue ? game::globals.forecolor.b() - 15 : game::globals.forecolor.b();

	grad = Color(red, green, blue, 245);

	draw.gradient_verticle(a.x, a.y, Location, 7, game::globals.forecolor, grad);

	draw.outline(a.x, a.y, m_iWidth, 7, Color(2, 2, 2, 255));

	char buffer[24];
	const char* format;
	if (Format == FORMAT_DECDIG2)
		sprintf_s(buffer, "%.2f%s", Value, extension );
	else if (Format == FORMAT_DECDIG1)
		sprintf_s(buffer, "%.1f%s", Value, extension );
	else if (Format == FORMAT_INT)
		sprintf_s(buffer, "%1.0f%s", Value, extension);

	RECT txtSize = draw.get_text_size(buffer, draw.fonts.menu_bold);
	draw.text(a.x + Location - (txtSize.right / 2), a.y + 7 - (txtSize.bottom / 2), buffer, draw.fonts.menu_bold, Color(180, 180, 180, 245));
}

void CSlider::OnUpdate(){
	POINT a = GetAbsolutePos();
	m_iHeight = 15;

	if (DoDrag)
	{
		if (GUI.GetKeyState(VK_LBUTTON))
		{
			POINT m = GUI.GetMouse();
			float NewX;
			float Ratio;
			NewX = m.x - a.x;//-1
			if ( NewX < 0 )
				NewX = 0;
			if ( NewX > m_iWidth )
				NewX = m_iWidth;
			Ratio = NewX / float( m_iWidth );
			Value = Min + ( Max - Min )*Ratio;
		}
		else
		{
			DoDrag = false;
		}
	}
}

void CSlider::OnClick(){
	POINT a = GetAbsolutePos();
	RECT SliderRegion = { a.x, a.y, m_iWidth, 11 };
	if (GUI.IsMouseInRegion(SliderRegion))
	{
		DoDrag = true;
	}
}

float CSlider::GetValue()
{
	return Value;
}

void CSlider::SetValue(float v)
{
	Value = v;
}

void CSlider::SetBoundaries(float min, float max)
{
	Min = min; Max = max;
}

void CSlider::SetFormat(SliderFormat type)
{
	Format = type;
}
#pragma endregion Implementations of the Slider functions

#pragma region ItemSelector
CItemSelector::CItemSelector()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_SaveFile;
	FileControlType = UIControlTypes::UIC_ItemSelector;
	m_iHeight = 16;
	Type = ITEMSELECTOR_INT;
}

void CItemSelector::Draw(bool hover)
{
	POINT a = GetAbsolutePos();

	//Minus

	RECT rMinus = { a.x, a.y, 16, m_iHeight };

	draw.outline(a.x - 1, a.y - 1, 18, m_iHeight + 2, Color(0, 0, 0, 255));
	draw.rect(a.x, a.y, 16, m_iHeight, Color(39, 39, 39, 255));

	if (GUI.IsMouseInRegion(rMinus) && GetAsyncKeyState(VK_LBUTTON))
		draw.gradient_verticle(a.x, a.y, 16, m_iHeight, Color(55, 55, 55, 255), Color(40, 40, 40, 245));

	RECT txtSizeMinus = draw.get_text_size("-", draw.fonts.menu);
	draw.text(a.x + (rMinus.right / 2) - (txtSizeMinus.right / 2),
		a.y + (m_iHeight / 2) - (txtSizeMinus.bottom / 2),
		XorStr("-"),
		draw.fonts.menu,
		Color(205, 205, 205, 245));

	//Main

	draw.outline(a.x + 19, a.y - 1, m_iWidth - 42, m_iHeight + 2, Color(0, 0, 0, 255));
	draw.gradient_verticle(a.x + 20, a.y, m_iWidth - 40, m_iHeight, Color(55, 55, 55, 255), Color(40, 40, 40, 245));

	if (Type == ITEMSELECTOR_INT)
	{
		char buffer[24];
		sprintf_s(buffer, "%1.0f", Value);

		RECT txtValueSize = draw.get_text_size(buffer, draw.fonts.menu);
		draw.text(a.x + (m_iWidth / 2) - (txtValueSize.right / 2),
			a.y + (m_iHeight / 2) - (txtValueSize.bottom / 2),
			buffer,
			draw.fonts.menu,
			Color(245, 245, 245, 245));
	}
	else if (Type == ITEMSELECTOR_STRING)
	{
		std::string text = Items[Value];
		RECT txtValueSize = draw.get_text_size(text.c_str(), draw.fonts.menu);
		draw.text(a.x + (m_iWidth / 2) - (txtValueSize.right / 2),
			a.y + (m_iHeight / 2) - (txtValueSize.bottom / 2),
			text.c_str(),
			draw.fonts.menu,
			Color(245, 245, 245, 245));
	}

	//Plus

	RECT rPlus = { a.x + (m_iWidth - 16), a.y, 16, m_iHeight };

	draw.outline(a.x + (m_iWidth - 16) - 1, a.y - 1, 18, m_iHeight + 2, Color(0, 0, 0, 255));
	draw.rect(a.x + (m_iWidth - 16), a.y, 16, m_iHeight, Color(39, 39, 39, 255));

	if (GUI.IsMouseInRegion(rPlus) && GetAsyncKeyState(VK_LBUTTON))
		draw.gradient_verticle(a.x + (m_iWidth - 16), a.y, 16, m_iHeight, Color(55, 55, 55, 255), Color(40, 40, 40, 245));

	RECT txtSizePlus = draw.get_text_size("+", draw.fonts.menu);
	draw.text(a.x + (m_iWidth - 8) - (txtSizePlus.right / 2),
		a.y + (m_iHeight / 2) - (txtSizePlus.bottom / 2),
		"+",
		draw.fonts.menu,
		Color(245, 245, 245, 245));

}

void CItemSelector::Manager()
{
	if (UpdateType == UPDATE_LEFT)
	{
		if (Value - 1 < Min)
			return;
		else
			Value -= 1;
	}
	else if (UpdateType == UPDATE_RIGHT)
	{
		if (Value + 1 > Max)
			return;
		else
			Value += 1;
	}

	if (!IsHolding)
	{
		UpdateType = UPDATE_NONE;
	}
}

void CItemSelector::OnUpdate()
{
	Manager();

	m_iHeight = 16;

	POINT a = GetAbsolutePos();

	RECT rMinus = { a.x, a.y, 16, m_iHeight };
	RECT rPlus = { a.x + (m_iWidth - 16), a.y, 16, m_iHeight };

	if (GUI.IsMouseInRegion(rMinus) && GetAsyncKeyState(VK_LBUTTON))
	{
		preTicks += 1;
		if (preTicks > 125)
		{
			UpdateType = UPDATE_LEFT;
			IsHolding = true;
		}
	}
	else if (GUI.IsMouseInRegion(rPlus) && GetAsyncKeyState(VK_LBUTTON))
	{
		preTicks += 1;
		if (preTicks > 125)
		{
			UpdateType = UPDATE_RIGHT;
			IsHolding = true;
		}
	}
	else
		preTicks = 0;

	IsHolding = false;
}

void CItemSelector::OnClick()
{
	POINT a = GetAbsolutePos();

	RECT rMinus = { a.x, a.y, 16, m_iHeight };
	RECT rPlus = { a.x + (m_iWidth - 16), a.y, 16, m_iHeight };

	if (GUI.IsMouseInRegion(rMinus))
		UpdateType = UPDATE_LEFT;
	else if (GUI.IsMouseInRegion(rPlus))
		UpdateType = UPDATE_RIGHT;
}

float CItemSelector::GetInt()
{
	return Value;
}

void CItemSelector::SetValue(float value)
{
	Value = value;
}

void CItemSelector::SetMin(float min)
{
	Min = min;
}

void CItemSelector::SetMax(float max)
{
	Max = max;
}

void CItemSelector::AddItem(std::string name)
{
	Items.push_back(name);
}

void CItemSelector::SetType(ItemSelector_Type type)
{
	Type = type;
}

#pragma endregion Implementations of the Item Selector functions

#pragma region KeyBinders

char* KeyStrings[254] = { "[ _ ]", "[M1]", "[M2]", "[BRK]", "[M3]", "[M4]", "[M5]",
"[ _ ]", "[BSPC]", "[TAB]", "[ _ ]", "[ _ ]", "[ _ ]", "[ENTER]", "[ _ ]", "[ _ ]", "[SHI]",
"[CTRL]", "[ALT]","[PAU]","[CAPS]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]",
"[ESC]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[SPACE]","[PGUP]", "[PGDOWN]", "[END]", "[HOME]", "[LEFT]",
"[UP]", "[RIGHT]", "[DOWN]", "[ _ ]", "[PRNT]", "[ _ ]", "[PRTSCR]", "[INS]","[DEL]", "[ _ ]", "[0]", "[1]",
"[2]", "[3]", "[4]", "[5]", "[6]", "[7]", "[8]", "[9]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]",
"[ _ ]", "[A]", "[B]", "[C]", "[D]", "[E]", "[F]", "[G]", "[H]", "[I]", "[J]", "[K]", "[L]", "[M]", "[N]", "[O]", "[P]", "[Q]", "[R]", "[S]", "[T]", "[U]",
"[V]", "[W]", "[X]","[Y]", "[Z]", "[LFTWIN]", "[RGHTWIN]", "[ _ ]", "[ _ ]", "[ _ ]", "[NUM0]", "[NUM1]",
"[NUM2]", "[NUM3]", "[NUM4]", "[NUM5]", "[NUM6]","[NUM7]", "[NUM8]", "[NUM9]", "[*]", "[+]", "[_]", "[-]", "[.]", "[/]", "[F1]", "[F2]", "[F3]",
"[F4]", "[F5]", "[F6]", "[F7]", "[F8]", "[F9]", "[F10]", "[F11]", "[F12]","[F13]", "[F14]", "[F15]", "[F16]", "[F17]", "[F18]", "[F19]", "[F20]", "[F21]",
"[F22]", "[F23]", "[F24]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]","[ _ ]", "[ _ ]", "[ _ ]",
"[NUM LOCK]", "[SCROLL LOCK[", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]",
"[ _ ]", "[ _ ]","[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[LSHFT]", "[RSHFT]", "[LCTRL]",
"[RCTRL]", "[LMENU]", "[RMENU]", "[ _ ]","[ _ ]", "[ _ ]","[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]",
"[ _ ]", "[ _ ]", "[ _ ]", "[NTRK]", "[PTRK]", "[STOP]", "[PLAY]", "[ _ ]", "[ _ ]",
"[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[;]", "[+]", "[,]", "[-]", "[.]", "[/?]", "[~]", "[ _ ]", "[ _ ]",
"[ _ ]", "[ _ ]","[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]",
"[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]","[ _ ]",
"[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[{]", "[\\|]", "[}]", "['\"]", "[ _ ]",
"[ _ ]", "[ _ ]", "[ _ ]","[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]",
"[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]",
"[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]", "[ _ ]",
"[ _ ]", "[ _ ]" };

CKeyBind::CKeyBind()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_SaveFile;
	m_iHeight = 12;
	FileControlType = UIControlTypes::UIC_KeyBind;
}

void CKeyBind::Draw(bool hover)
{
	POINT a = GetAbsolutePos();
	if (this->Getting_New_Key)
	{
		draw.text(a.x, a.y, "[ _ ]", draw.fonts.menu, Color(110, 110, 110, 255));
	}
	else
	{
		if (key == -1)
			draw.text(a.x, a.y, "[ _ ]", draw.fonts.menu, Color(110, 110, 110, 255));
		else
		{
			char* NameOfKey = KeyStrings[key];
			draw.text(a.x, a.y, NameOfKey, draw.fonts.menu, Color(110, 110, 110, 255));
		}
	}
	
}

void CKeyBind::OnUpdate(){
	m_iHeight = 13;
	RECT text_area;
	if (key == -1)
		text_area = draw.get_text_size("[ _ ]", draw.fonts.menu);
	else
		text_area = draw.get_text_size(text, draw.fonts.menu);
	m_iWidth = text_area.right;
	POINT a = GetAbsolutePos();
	if (Getting_New_Key)
	{
		for (int i = 0; i < 255; i++)
		{
			if (GUI.GetKeyPress(i))
			{
				if (i == VK_ESCAPE)
				{
					Getting_New_Key = false;
					key = -1;
					text = "[ _ ]";
					return;
				}

				key = i;
				Getting_New_Key = false;
				text = KeyStrings[i];
				return;
			}
		}
	}
}

void CKeyBind::OnClick(){
	POINT a = GetAbsolutePos();
	if (!Getting_New_Key)
		Getting_New_Key = true;
}

int CKeyBind::GetKey()
{
	return key;
}

void CKeyBind::SetKey(int k)
{
	key = k;
	text = KeyStrings[k];
}

#pragma endregion Implementations of the KeyBind Control functions

#pragma region Button
CButton::CButton()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable;
	FileControlType == UIControlTypes::UIC_Button;
	Text = "Default";
	m_iHeight = 25;
	CallBack = nullptr;
	FileIdentifier = "Default";
}

void CButton::Draw(bool hover)
{
	POINT a = GetAbsolutePos();
	if (hover)
		draw.gradient_verticle(a.x, a.y, m_iWidth, m_iHeight, Color(55, 55, 55, 255), Color(55, 55, 55, 255));
	else
		draw.gradient_verticle(a.x, a.y, m_iWidth, m_iHeight, Color(45, 45, 45, 255), Color(45, 45, 45, 255));

	draw.outline(a.x, a.y, m_iWidth, m_iHeight, Color(2, 2, 2, 255));

	RECT TextSize = draw.get_text_size(Text.c_str(), draw.fonts.menu_bold);
	int TextX = a.x + (m_iWidth / 2) - (TextSize.right / 2);
	int TextY = a.y + (m_iHeight / 2) - (TextSize.bottom / 2);

	draw.text(TextX, TextY, Text.c_str(), draw.fonts.menu_bold, Color(180, 180, 180, 200));
}

void CButton::SetText(std::string text)
{
	Text = text;
}

void CButton::SetCallback(CButton::ButtonCallback_t callback)
{
	CallBack = callback;
}

void CButton::OnUpdate()
{
	m_iHeight = 25;
}

void CButton::OnClick()
{
	if (CallBack)
		CallBack();
}
#pragma endregion Implementations of the Button functions

#pragma region ComboBox
CComboBox::CComboBox()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_Focusable | UIFlags::UI_SaveFile;
	m_iHeight = 20;
	FileControlType = UIControlTypes::UIC_ComboBox;
}

void CComboBox::Draw(bool hover)
{
	POINT a = GetAbsolutePos();
	RECT Region = { a.x, a.y, m_iWidth, 20 };
	draw.gradient_verticle(a.x, a.y, m_iWidth, 20, Color(45, 45, 45, 255), Color(45, 45, 45, 255));
	if (GUI.IsMouseInRegion(Region)) draw.gradient_verticle(a.x, a.y, m_iWidth, 20, Color(55, 55, 55, 255), Color(55, 55, 55, 255));
	draw.outline(a.x, a.y, m_iWidth, 20, Color(2, 2, 2, 255));

	if (Items.size() > 0)
	{
		RECT txtSize = draw.get_text_size(GetItem().c_str(), draw.fonts.menu);
		draw.text(a.x + 10, a.y + (Region.bottom / 2) - (txtSize.bottom / 2), GetItem().c_str(), draw.fonts.menu, Color(180, 180, 180, 245));

		if (IsOpen)
		{
			draw.gradient_verticle(a.x, a.y + 20, m_iWidth, Items.size() * 20, Color(45, 45, 45, 255), Color(45, 45, 45, 255));

			for (int i = 0; i < Items.size(); i++)
			{
				RECT ItemRegion = { a.x, a.y + 17 + i * 20, m_iWidth, 20 };

				if (GUI.IsMouseInRegion(ItemRegion))
				{
					draw.gradient_verticle(a.x, a.y + 20 + i * 20, m_iWidth, 20, Color(35, 35, 35, 255), Color(35, 35, 35, 255));
				}

				RECT control_textsize = draw.get_text_size(Items[i].c_str(), draw.fonts.menu);
				if (i == SelectedIndex)
					draw.text(a.x + 10, a.y + 20 + (i * 20) + 10 - (control_textsize.bottom / 2), Items[i].c_str(), draw.fonts.menu, game::globals.forecolor);
				else
					draw.text(a.x + 10, a.y + 20 + (i * 20) + 10 - (control_textsize.bottom / 2), Items[i].c_str(), draw.fonts.menu, Color(180, 180, 180, 245));
			}
			draw.outline(a.x, a.y + 20, m_iWidth, Items.size() * 20, Color(2, 2, 2, 255));
		}
	}
	Vertex_t Verts2[3];
	Verts2[0].m_Position.x = a.x + m_iWidth - 10;
	Verts2[0].m_Position.y = a.y + 9;
	Verts2[1].m_Position.x = a.x + m_iWidth - 5;
	Verts2[1].m_Position.y = a.y + 9;
	Verts2[2].m_Position.x = a.x + m_iWidth - 7.5;
	Verts2[2].m_Position.y = a.y + 12;

	draw.polygon(3, Verts2, Color(92, 92, 92, 255));
}

void CComboBox::AddItem(std::string text)
{
	Items.push_back(text);
	SelectedIndex = 0;
}

void CComboBox::OnUpdate()
{
	if (IsOpen)
	{
		m_iHeight = 20 + 20 * Items.size();

		if (parent->GetFocus() != this)
			IsOpen = false;
	}
	else
	{
		m_iHeight = 20;
	}
		
}

void CComboBox::OnClick()
{
	POINT a = GetAbsolutePos();
	RECT Region = { a.x, a.y, m_iWidth, 20 };

	if (IsOpen)
	{
		// If we clicked one of the items(Not in the top bar)
		if (!GUI.IsMouseInRegion(Region))
		{
			// Draw the items
			for (int i = 0; i < Items.size(); i++)
			{
				RECT ItemRegion = { a.x, a.y + 20 + i * 20, m_iWidth, 20 };

				// Hover
				if (GUI.IsMouseInRegion(ItemRegion))
				{
					SelectedIndex = i;
				}
			}
		}

		// Close the drop down
		IsOpen = false;
	}
	else
	{
		IsOpen = true;
	}
	
}

int CComboBox::GetIndex()
{
	return SelectedIndex; 
}

void CComboBox::SetIndex(int index)
{
	SelectedIndex = index;
}

std::string CComboBox::GetItem()
{
	if (SelectedIndex >= 0 && SelectedIndex < Items.size())
	{
		return Items[SelectedIndex];
	}

	return "";
}

void CComboBox::SelectIndex(int idx)
{
	if (idx >= 0 && idx < Items.size())
	{
		SelectedIndex = idx;
	}
}

#pragma endregion Implementations of the ComboBox functions

#pragma region MultiBox
CMultiBox::CMultiBox()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_Focusable | UIFlags::UI_SaveFile;
	m_iHeight = 40;
	FileControlType = UIControlTypes::UIC_MultiBox;
}

void CMultiBox::Draw(bool hover)
{
	POINT a = GetAbsolutePos();

	draw.outline(a.x, a.y, m_iWidth, (items.size() * 16) + 19, Color(2, 2, 2, 255));
	draw.gradient_verticle(a.x + 1, a.y + 1, m_iWidth - 2, 24, Color(35, 35, 35, 255), Color(39, 39, 39, 255));

	RECT TitleSize = draw.get_text_size(title, draw.fonts.menu);
	int title_x = a.x + (m_iWidth / 2) - (TitleSize.right / 2);
	int title_y = a.y + 8 - (TitleSize.bottom / 2);
	draw.text(title_x, title_y, title, draw.fonts.menu, Color(255, 255, 255, 255));

	draw.gradient_verticle(a.x + 1, a.y + 18, m_iWidth - 2, items.size() * 16, Color(55, 55, 55, 255), Color(40, 40, 40, 245));

	for (int i = 0; i < items.size(); i++)
	{
		MultiBoxItem item = items[i];
		RECT txtsize = draw.get_text_size(item.text, draw.fonts.menu);
		int item_x = a.x + (m_iWidth / 2) - (txtsize.right / 2);
		int item_y = a.y + 19 + (i * 16) - (txtsize.bottom / 2) + 7;
		if (!item.bselected)
			draw.text(item_x, item_y, item.text, draw.fonts.menu, Color(245, 245, 245, 245));
		else
			draw.text(item_x, item_y, item.text, draw.fonts.menu, game::globals.forecolor);
	}
}

void CMultiBox::OnUpdate()
{
	m_iHeight = (items.size() * 16) + 19;

}

void CMultiBox::OnClick()
{
	POINT a = GetAbsolutePos();
	for (int i = 0; i < items.size(); i++)
	{
		RECT area = { a.x, a.y + i * 16 + 19, m_iWidth, 16 };
		if (GUI.IsMouseInRegion(area))
		{
			items[i].bselected = !items[i].bselected;
		}
	}
}

void CMultiBox::SetTitle(const char* tl)
{
	title = tl;
}
#pragma endregion Implementations of the MultiBox functions

#pragma region TextField
char* KeyDigitsLowercase[254] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x",
"y", "z", nullptr, nullptr, nullptr, nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6",
"7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, ";", "+", ",", "-", ".", "/?", "~", nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "[", "\\", "]", "'", nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

char* KeyDigitsCapital[254] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X",
"Y", "Z", nullptr, nullptr, nullptr, nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6",
"7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, ";", "+", ",", "-", ".", "?", "~", nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "{", "|", "}", "\"", nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

CTextField::CTextField()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_SaveFile;
	FileControlType = UIControlTypes::UIC_TextField;
	m_iHeight = 12;
}

std::string CTextField::getText()
{
	return text;
}

void CTextField::SetText(std::string stext)
{
	text = stext;
}

void CTextField::Draw(bool hover)
{
	POINT a = GetAbsolutePos();

	std::string drawn_text = "[";

	const char *cstr = text.c_str();

	drawn_text += cstr;

	if (IsGettingKey)
		drawn_text += "_";

	drawn_text += "]";

	draw.text(a.x, a.y, drawn_text.c_str(), draw.fonts.menu, Color(244, 244, 244, 255));
}

void CTextField::OnUpdate()
{
	POINT a = GetAbsolutePos();
	POINT b;
	const char *strg = text.c_str();

	if (IsGettingKey)
	{
		b = GetAbsolutePos();
		for (int i = 0; i < 255; i++)
		{

			if (GUI.GetKeyPress(i))
			{
				if (i == VK_ESCAPE || i == VK_RETURN || i == VK_INSERT)
				{
					IsGettingKey = false;
					return;
				}

				if (i == VK_BACK && strlen(strg) != 0)
				{
					text = text.substr(0, strlen(strg) - 1);
				}

				if (strlen(strg) < 20 && i != NULL && KeyDigitsCapital[i] != nullptr)
				{
					if (GetAsyncKeyState(VK_SHIFT))
					{
						text = text + KeyDigitsCapital[i];
					}
					else
					{
						text = text + KeyDigitsLowercase[i];
					}
					return;
				}

				if (strlen(strg) < 20 && i == 32)
				{
					text = text + " ";
					return;
				}
			}
		}
	}
}

void CTextField::OnClick()
{
	POINT a = GetAbsolutePos();
	if (!IsGettingKey)
	{
		IsGettingKey = true;
	}
}
#pragma endregion Implementations of the Text Field functions

#define LIST_ITEM_HEIGHT 16
#define LIST_SCROLL_WIDTH 8

#pragma region ListBox
CListBox::CListBox()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_Focusable | UIFlags::UI_SaveFile;
	SelectedIndex = 0;
	FileControlType = UIControlTypes::UIC_ListBox;
}

void CListBox::Draw(bool hover)
{
	int ItemsToDraw = m_iHeight / LIST_ITEM_HEIGHT;
	POINT a = GetAbsolutePos();

	draw.rect(a.x + 1, a.y + 1, m_iWidth - 2, m_iHeight - 2, Color(90, 90, 90, 1));

	//Top Left
	draw.line(a.x, a.y, a.x + m_iWidth - 2, a.y, Color(48, 48, 48, 255));
	draw.line(a.x - 1, a.y - 1, a.x + (m_iWidth / 2) - 1, a.y - 1, Color(0, 0, 0, 255));

	//Top Right
	draw.line(a.x + (m_iWidth / 2) + 2, a.y, a.x + m_iWidth, a.y, Color(48, 48, 48, 255));
	draw.line(a.x + (m_iWidth / 2) + 2, a.y - 1, a.x + m_iWidth + 1, a.y - 1, Color(0, 0, 0, 255));

	//Left
	draw.line(a.x, a.y, a.x, a.y + m_iHeight, Color(49, 49, 49, 255));
	draw.line(a.x - 1, a.y, a.x - 1, a.y + m_iHeight, Color(0, 0, 0, 255));

	//Bottom
	draw.line(a.x, a.y + m_iHeight, a.x + m_iWidth, a.y + m_iHeight, Color(48, 48, 48, 255));
	draw.line(a.x - 1, a.y + m_iHeight + 1, a.x + m_iWidth + 2, a.y + m_iHeight + 1, Color(0, 0, 0, 255));

	//Right
	draw.line(a.x + m_iWidth, a.y, a.x + m_iWidth, a.y + m_iHeight + 1, Color(48, 48, 48, 255));
	draw.line(a.x + m_iWidth + 1, a.y, a.x + m_iWidth + 1, a.y + m_iHeight + 1, Color(0, 0, 0, 255));

	if (Items.size() > 0)
	{
		int drawnItems = 0;
		for (int i = ScrollTop; (i < Items.size() && drawnItems < ItemsToDraw); i++)
		{
			Color textColor = Color(92, 92, 92, 255);
			RECT ItemRegion = { a.x + 1, a.y + 1 + drawnItems * 16, m_iWidth - LIST_SCROLL_WIDTH - 2 , 16 };

			if (i == SelectedIndex)
			{
				textColor = Color(245, 245, 245, 245);

				bool bSetRed = false;
				bool bSetGreen = false;
				bool bSetBlue = false;
				if (game::globals.forecolor.r() >= 15)
					bSetRed = true;
				if (game::globals.forecolor.g() >= 15)
					bSetGreen = true;
				if (game::globals.forecolor.b() >= 15)
					bSetBlue = true;

				float red = bSetRed ? game::globals.forecolor.r() - 15 : game::globals.forecolor.r();
				float green = bSetGreen ? game::globals.forecolor.g() - 15 : game::globals.forecolor.g();
				float blue = bSetBlue ? game::globals.forecolor.b() - 15 : game::globals.forecolor.b();

				draw.gradient_verticle(ItemRegion.left, ItemRegion.top, ItemRegion.right, ItemRegion.bottom, game::globals.forecolor, Color(red, green, blue, 255));
			}
			else if (GUI.IsMouseInRegion(ItemRegion))
			{
				textColor = Color(245, 245, 245, 245);
				draw.rect(ItemRegion.left, ItemRegion.top, ItemRegion.right, ItemRegion.bottom, Color(92, 92, 92, 255));
			}

			draw.text(ItemRegion.left + 4, ItemRegion.top + 2, Items[i].c_str(), draw.fonts.menu, textColor);
			drawnItems++;
		}

		// Ratio of how many visible to how many are hidden
		float sizeRatio = float(ItemsToDraw) / float(Items.size());
		if (sizeRatio > 1.f) sizeRatio = 1.f;
		float posRatio = float(ScrollTop) / float(Items.size());
		if (posRatio > 1.f) posRatio = 1.f;

		sizeRatio *= m_iHeight;
		posRatio *= m_iHeight;

		draw.rect(a.x + m_iWidth - LIST_SCROLL_WIDTH, a.y + posRatio, LIST_SCROLL_WIDTH, sizeRatio, Color(52, 52, 52, 255));
	}

}

void CListBox::AddItem(std::string text, int value)
{
	Items.push_back(text);
	Values.push_back(value);
}

void CListBox::SetText(std::string text)
{
	Text = text;
}

void CListBox::OnClick()
{
	int ItemsToDraw = m_iHeight / LIST_ITEM_HEIGHT;
	POINT a = GetAbsolutePos();

	// Check the items
	if (Items.size() > 0)
	{
		int drawnItems = 0;
		for (int i = ScrollTop; (i < Items.size() && drawnItems < ItemsToDraw); i++)
		{
			Color textColor = Color(92, 92, 92, 255);
			RECT ItemRegion = { a.x + 1, a.y + 1 + drawnItems * 16, m_iWidth - LIST_SCROLL_WIDTH - 2 , 16 };
			if (GUI.IsMouseInRegion(ItemRegion))
			{
				SelectItem(i);
				return;
			}
			drawnItems++;
		}
	}
}

void CListBox::OnUpdate()
{
	int ItemsToDraw = m_iHeight / LIST_ITEM_HEIGHT;
	POINT a = GetAbsolutePos();

	// Did we click in the scrollbar??
	RECT Scroll = { a.x + m_iWidth - LIST_SCROLL_WIDTH , a.y + 1, LIST_SCROLL_WIDTH - 2 ,m_iHeight };

	if (GUI.IsMouseInRegion(Scroll) && GetAsyncKeyState(VK_LBUTTON)) dragging = true;
	else if (!GetAsyncKeyState(VK_LBUTTON) && dragging) dragging = false;

	if (dragging)
	{
		// Ratio of how many visible to how many are hidden
		float ratio = float(ItemsToDraw) / float(Items.size());
		POINT m = GUI.GetMouse();
		m.y -= a.y;

		float sizeRatio = float(ItemsToDraw) / float(Items.size());
		sizeRatio *= m_iHeight;
		float heightDelta = m.y + sizeRatio - m_iHeight;
		if (heightDelta > 0)
			m.y -= heightDelta;

		float mPosRatio = float(m.y) / float(m_iHeight);
		ScrollTop = mPosRatio*Items.size();
		if (ScrollTop < 0)
			ScrollTop = 0;
	}
}

void CListBox::SetHeightInItems(int items)
{
	m_iHeight = items*LIST_ITEM_HEIGHT;
}

void CListBox::SetHeightPlayersTab(int items)
{
	m_iHeight = items * 15;
}

std::string CListBox::GetItem()
{
	if (SelectedIndex >= 0 && SelectedIndex < Items.size())
	{
		return Items[SelectedIndex];
	}

	return "Error";
}

#pragma endregion Implementations of the ListBox functions

CColorSelector::CColorSelector()
{
	m_Flags = UIFlags::UI_Drawable | UIFlags::UI_Clickable | UIFlags::UI_Focusable | UIFlags::UI_SaveFile;
	m_iHeight = 10;
	FileControlType = UIControlTypes::UIC_ColorSelector;
}

void CColorSelector::Draw(bool hover)
{
	POINT a = GetAbsolutePos();

	Color preview;
	preview.SetColor(color[0], color[1], color[2], color[3]);

	draw.rect(is_open && set_new_pos ? a.x + 194 : a.x, a.y, 16, 10, preview);
	draw.outline(is_open && set_new_pos ? a.x + 194 : a.x, a.y, 16, 10, Color(2, 2, 2, 255));

	if (is_open && set_new_pos)
	{
		int _x = a.x + 6;
		int _y = a.y + 17;
		int _width = 200;
		int _height = 200;

		draw.outline(_x - 6, _y - 6, _width + 12, _height + 12, darker_gray);
		draw.outlined_rect(_x - 5, _y - 5, _width + 10, _height + 10, lighter_gray, light_gray);
		draw.outlined_rect(_x, _y, _width, _height, lighter_gray, gray);
		_x += 5; _y += 5;
		draw.color_spectrum(_x, _y, 190, 190);
	}
}

void CColorSelector::OnUpdate() {
	POINT a = GetAbsolutePos();

	if (is_open && !toggle)
	{
		m_x -= 194;
		set_new_pos = true;
		toggle = true;
	}
	else if (!is_open && toggle)
	{
		m_x += 194;
		set_new_pos = false;
		toggle = false;
	}

	if (is_open && set_new_pos && GetAsyncKeyState(VK_LBUTTON))
	{
		int _x = a.x + 11;
		int _y = a.y + 22;
		RECT color_region = { _x, _y, 190, 190 };
		if (GUI.IsMouseInRegion(color_region))
		{
			color[0] = draw.color_spectrum_pen(_x, _y, 190, 190, Vector(GUI.GetMouse().x - _x, GUI.GetMouse().y - _y, 0)).r();
			color[1] = draw.color_spectrum_pen(_x, _y, 190, 190, Vector(GUI.GetMouse().x - _x, GUI.GetMouse().y - _y, 0)).g();
			color[2] = draw.color_spectrum_pen(_x, _y, 190, 190, Vector(GUI.GetMouse().x - _x, GUI.GetMouse().y - _y, 0)).b();
			color[3] = draw.color_spectrum_pen(_x, _y, 190, 190, Vector(GUI.GetMouse().x - _x, GUI.GetMouse().y - _y, 0)).a();
		}
	}

	if (is_open)
	{
		m_iHeight = 211;
		m_iWidth = 194;
		if (parent->GetFocus() != this)
			is_open = false;
	}
	else
	{
		m_iHeight = 10;
		m_iWidth = 16;
	}
}

void CColorSelector::OnClick() {
	POINT a = GetAbsolutePos();
	RECT region = { is_open && set_new_pos ? a.x + 200 : a.x, a.y, 16, 10 };
	if (GUI.IsMouseInRegion(region)) is_open = !is_open;
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class sdbfewb
{
public:
	string tjiahefihuogl;
	double lwozzo;
	int jkvkx;
	sdbfewb();
	int hwtivjrruertnrjbtsbrriode(bool enomjrdmpsy, bool clbbeuucdgwzn, bool rlrnc, double nkbxjryyzq, double mjuxjpechsvowv, int wyepkcypmpndeew, string hknkqougdzzt, int qfvxvhain);
	string yuwjkrdccjpiogvolynbhl(string zcywhho, bool amshyanxdna);
	void bhepanimtejfaqyeetrc(double kczpxloytyysb, double wenokmbhh, double dizxhmyht);

protected:
	bool bkczoyrkmrtgk;
	string lcelmyjrotx;
	double fumxrvjfh;
	bool pwxiwpvuhyvts;
	bool aiqzuf;

	bool nsqqlitniiepvytwijambd(bool pcjnkvqnhjtxta, double mvwxunvaj, double qatrgavyqcmsbd);
	bool loaugbetsn(string kawlozlyhfjg, int hywjiufl, bool xfpvbwbgrmr);
	bool pjpftfnobqdqukduzadyhl(int rmirhxymjcc, int crqtdfgkcwskfk);
	int amcgrlmnquzgduotunwonjvra(string pxisimdsupbj, bool dmrfcijorx, int ztqcskqitrkmv, double cwlptn, int lnkcqqlruasmfjg, int cpliy);
	int kvdmsoaeicgpqjnsldnpuror(double vtzzetshvry, int wdwcjtlbmdo, double ozjusurkmmnz, double bqcqjzgmqwona, double mbhbtgaqq, double ctobrqegalrdh, int fihtjfpxqlckb, double ibidhmseyvwg, double sxvtfdkqgiubkg);
	string folqpnhgblb(double bhqtwxu, bool bdeav, int fcmxgzuqplojybb, bool hjpyncimpigjfe, string fyfwj);
	double nybhhtjnpajbpmwftmhsjfbno(string lkcbkqwgs, int hkesxrrhor, bool zhccx, double valccg, int fadkau, string tjbtu, bool pdmuc, bool bmyzyvmbuvdj, int wtqghfozpxilpqa, double zgvyc);
	void ajtdbdlodgwwjacpdepft(string zykfp);

private:
	bool ddecopomopmwncf;
	double qrmcnqagzeanqzv;

	double pkoxvmqzypbhipdgd();
	bool kvppzlguapxawbmzcrxqcef(int lskuvta, bool nrqirmlholrwch, string srstzjvafj, int gorwmt, double urhsfnrr, int oehlvri, string zhrjwvadhw, bool fzszs, int qeeyfaiqa, string erbib);
	bool fukvvzvaauhaoxzheprmu(string rbnynedrlcz, int gscptjwnjvudl, bool ctvth, double gqwbwamz);

};



double sdbfewb::pkoxvmqzypbhipdgd()
{
	double vlayulph = 20118;
	int ieyznervrd = 7788;
	string xqynmdktoih = "atfsxjwnzjut";
	bool wtvlzu = false;
	int ekqxjfzxhxogdc = 4006;
	bool poowcfc = true;
	int lpxkqo = 5829;
	if(false == false)
	{
		int eprfgu;
		for(eprfgu = 83; eprfgu > 0; eprfgu--)
		{
			continue;
		}
	}
	if(4006 == 4006)
	{
		int dijevgbb;
		for(dijevgbb = 25; dijevgbb > 0; dijevgbb--)
		{
			continue;
		}
	}
	if(false != false)
	{
		int xcrw;
		for(xcrw = 18; xcrw > 0; xcrw--)
		{
			continue;
		}
	}
	if(false != false)
	{
		int ovihi;
		for(ovihi = 41; ovihi > 0; ovihi--)
		{
			continue;
		}
	}
	if(4006 != 4006)
	{
		int cs;
		for(cs = 66; cs > 0; cs--)
		{
			continue;
		}
	}
	return 90088;
}

bool sdbfewb::kvppzlguapxawbmzcrxqcef(int lskuvta, bool nrqirmlholrwch, string srstzjvafj, int gorwmt, double urhsfnrr, int oehlvri, string zhrjwvadhw, bool fzszs, int qeeyfaiqa, string erbib)
{
	string iinhnddrwrprd = "jklnxrwbncyaxnjcanuphbs";
	bool osbdhzj = true;
	int hpjhyajclcdyi = 1163;
	if(string("jklnxrwbncyaxnjcanuphbs") == string("jklnxrwbncyaxnjcanuphbs"))
	{
		int yufbds;
		for(yufbds = 11; yufbds > 0; yufbds--)
		{
			continue;
		}
	}
	if(1163 != 1163)
	{
		int zoubyptce;
		for(zoubyptce = 95; zoubyptce > 0; zoubyptce--)
		{
			continue;
		}
	}
	if(1163 != 1163)
	{
		int zgpocj;
		for(zgpocj = 46; zgpocj > 0; zgpocj--)
		{
			continue;
		}
	}
	if(1163 == 1163)
	{
		int goyqd;
		for(goyqd = 81; goyqd > 0; goyqd--)
		{
			continue;
		}
	}
	return false;
}

bool sdbfewb::fukvvzvaauhaoxzheprmu(string rbnynedrlcz, int gscptjwnjvudl, bool ctvth, double gqwbwamz)
{
	int ldztriae = 2171;
	string mjbzurcj = "eloilchbaubkqxzdnujsqfuwiujphmihdvvqjcr";
	double derehcewj = 7795;
	bool fjkxqzefh = true;
	string hlouxyqb = "vghzexwzohxgdxapqaoy";
	double ujswvw = 18054;
	if(7795 != 7795)
	{
		int cqajzue;
		for(cqajzue = 76; cqajzue > 0; cqajzue--)
		{
			continue;
		}
	}
	if(string("eloilchbaubkqxzdnujsqfuwiujphmihdvvqjcr") == string("eloilchbaubkqxzdnujsqfuwiujphmihdvvqjcr"))
	{
		int rehyhvbzi;
		for(rehyhvbzi = 64; rehyhvbzi > 0; rehyhvbzi--)
		{
			continue;
		}
	}
	if(true == true)
	{
		int hl;
		for(hl = 77; hl > 0; hl--)
		{
			continue;
		}
	}
	if(string("vghzexwzohxgdxapqaoy") != string("vghzexwzohxgdxapqaoy"))
	{
		int pbdntjw;
		for(pbdntjw = 28; pbdntjw > 0; pbdntjw--)
		{
			continue;
		}
	}
	return false;
}

bool sdbfewb::nsqqlitniiepvytwijambd(bool pcjnkvqnhjtxta, double mvwxunvaj, double qatrgavyqcmsbd)
{
	double dznmsdbfzawq = 17995;
	if(17995 == 17995)
	{
		int bo;
		for(bo = 83; bo > 0; bo--)
		{
			continue;
		}
	}
	if(17995 == 17995)
	{
		int vfqe;
		for(vfqe = 48; vfqe > 0; vfqe--)
		{
			continue;
		}
	}
	if(17995 == 17995)
	{
		int rsed;
		for(rsed = 62; rsed > 0; rsed--)
		{
			continue;
		}
	}
	return true;
}

bool sdbfewb::loaugbetsn(string kawlozlyhfjg, int hywjiufl, bool xfpvbwbgrmr)
{
	return false;
}

bool sdbfewb::pjpftfnobqdqukduzadyhl(int rmirhxymjcc, int crqtdfgkcwskfk)
{
	bool pxfroeiqvm = false;
	double kanadwuqsjasje = 44537;
	int uhphl = 2020;
	bool xklagybtobed = true;
	string komhgemqx = "mgiitdlqicirmrqfwqpoxwzjdktuejokbkzfiradldrrfavhzrqtwdpccilifi";
	string oqorejom = "jotwwbltzlagavid";
	string hiwsevlvi = "paqfwphfoutjphqwuqlqn";
	string hcqkgcsuwbb = "aexksuircseszfjkpgennwjxtisezlowakjjspiiwoklhwrhkkcreoulnj";
	int uwkdxvuxqsepy = 3778;
	int apoafteyene = 5554;
	if(string("jotwwbltzlagavid") == string("jotwwbltzlagavid"))
	{
		int sfiqed;
		for(sfiqed = 17; sfiqed > 0; sfiqed--)
		{
			continue;
		}
	}
	if(3778 != 3778)
	{
		int slti;
		for(slti = 48; slti > 0; slti--)
		{
			continue;
		}
	}
	if(string("aexksuircseszfjkpgennwjxtisezlowakjjspiiwoklhwrhkkcreoulnj") != string("aexksuircseszfjkpgennwjxtisezlowakjjspiiwoklhwrhkkcreoulnj"))
	{
		int kgn;
		for(kgn = 25; kgn > 0; kgn--)
		{
			continue;
		}
	}
	return true;
}

int sdbfewb::amcgrlmnquzgduotunwonjvra(string pxisimdsupbj, bool dmrfcijorx, int ztqcskqitrkmv, double cwlptn, int lnkcqqlruasmfjg, int cpliy)
{
	return 67463;
}

int sdbfewb::kvdmsoaeicgpqjnsldnpuror(double vtzzetshvry, int wdwcjtlbmdo, double ozjusurkmmnz, double bqcqjzgmqwona, double mbhbtgaqq, double ctobrqegalrdh, int fihtjfpxqlckb, double ibidhmseyvwg, double sxvtfdkqgiubkg)
{
	double ttsvjougihn = 38028;
	double lxldkfvdjfy = 77967;
	int cqjwasrx = 2139;
	string pveczkvfl = "tqvivuwkqfvonaihgmyjxarxsiesrwdgnrkuyhjgqxvmkqxoxctrqicqvhputpbqnypnhrvxxkmoekzywafrdxeqxgjxx";
	if(2139 == 2139)
	{
		int bggwwlx;
		for(bggwwlx = 2; bggwwlx > 0; bggwwlx--)
		{
			continue;
		}
	}
	if(string("tqvivuwkqfvonaihgmyjxarxsiesrwdgnrkuyhjgqxvmkqxoxctrqicqvhputpbqnypnhrvxxkmoekzywafrdxeqxgjxx") != string("tqvivuwkqfvonaihgmyjxarxsiesrwdgnrkuyhjgqxvmkqxoxctrqicqvhputpbqnypnhrvxxkmoekzywafrdxeqxgjxx"))
	{
		int kungjnfnw;
		for(kungjnfnw = 99; kungjnfnw > 0; kungjnfnw--)
		{
			continue;
		}
	}
	if(77967 == 77967)
	{
		int olfxlawo;
		for(olfxlawo = 3; olfxlawo > 0; olfxlawo--)
		{
			continue;
		}
	}
	if(77967 != 77967)
	{
		int avzzsaeery;
		for(avzzsaeery = 47; avzzsaeery > 0; avzzsaeery--)
		{
			continue;
		}
	}
	if(38028 != 38028)
	{
		int aliozk;
		for(aliozk = 39; aliozk > 0; aliozk--)
		{
			continue;
		}
	}
	return 74821;
}

string sdbfewb::folqpnhgblb(double bhqtwxu, bool bdeav, int fcmxgzuqplojybb, bool hjpyncimpigjfe, string fyfwj)
{
	string gmyybpckwqa = "npjsvxzounwoswnoatfoofeltdzjnqbyohd";
	int kuwdhx = 2622;
	int jjajkdpxmhmj = 348;
	string aetkkq = "yqtissvgdujxuiysqljkqwdapscpqedxyrhzqxoyjoxdwnxoccjqbwkmnwmmsynkkyseezlwtmwcndhinl";
	if(string("npjsvxzounwoswnoatfoofeltdzjnqbyohd") == string("npjsvxzounwoswnoatfoofeltdzjnqbyohd"))
	{
		int tsuodqp;
		for(tsuodqp = 73; tsuodqp > 0; tsuodqp--)
		{
			continue;
		}
	}
	if(string("yqtissvgdujxuiysqljkqwdapscpqedxyrhzqxoyjoxdwnxoccjqbwkmnwmmsynkkyseezlwtmwcndhinl") == string("yqtissvgdujxuiysqljkqwdapscpqedxyrhzqxoyjoxdwnxoccjqbwkmnwmmsynkkyseezlwtmwcndhinl"))
	{
		int hk;
		for(hk = 26; hk > 0; hk--)
		{
			continue;
		}
	}
	return string("xfpasnu");
}

double sdbfewb::nybhhtjnpajbpmwftmhsjfbno(string lkcbkqwgs, int hkesxrrhor, bool zhccx, double valccg, int fadkau, string tjbtu, bool pdmuc, bool bmyzyvmbuvdj, int wtqghfozpxilpqa, double zgvyc)
{
	double iermw = 247;
	if(247 == 247)
	{
		int okpugpii;
		for(okpugpii = 41; okpugpii > 0; okpugpii--)
		{
			continue;
		}
	}
	if(247 != 247)
	{
		int onstwoxkg;
		for(onstwoxkg = 0; onstwoxkg > 0; onstwoxkg--)
		{
			continue;
		}
	}
	if(247 == 247)
	{
		int vgqlxy;
		for(vgqlxy = 21; vgqlxy > 0; vgqlxy--)
		{
			continue;
		}
	}
	if(247 == 247)
	{
		int kqhqic;
		for(kqhqic = 66; kqhqic > 0; kqhqic--)
		{
			continue;
		}
	}
	return 42313;
}

void sdbfewb::ajtdbdlodgwwjacpdepft(string zykfp)
{
	bool pdnleqeekh = true;
	int oxydbmtvhethgfp = 3526;
	int rqkag = 143;
	double xgotqgjrtvla = 30465;
	string mxltarvhhhdgh = "hoxscydgnmgtgmnujmckdhkawcwpymksoejxizckwlabmsdaatzhzqjcsewwhxekfypqjarckgvvbuzf";
	if(string("hoxscydgnmgtgmnujmckdhkawcwpymksoejxizckwlabmsdaatzhzqjcsewwhxekfypqjarckgvvbuzf") == string("hoxscydgnmgtgmnujmckdhkawcwpymksoejxizckwlabmsdaatzhzqjcsewwhxekfypqjarckgvvbuzf"))
	{
		int jesxp;
		for(jesxp = 11; jesxp > 0; jesxp--)
		{
			continue;
		}
	}

}

int sdbfewb::hwtivjrruertnrjbtsbrriode(bool enomjrdmpsy, bool clbbeuucdgwzn, bool rlrnc, double nkbxjryyzq, double mjuxjpechsvowv, int wyepkcypmpndeew, string hknkqougdzzt, int qfvxvhain)
{
	bool gqnfa = true;
	string vtujw = "ppvwosqieykqhpmgwwgyuvkyofdsdihevfkwuhqcxjupijnhbunsfdecpgpeiuyhtxhqsbolnipqqxniujdvfe";
	int ohelyg = 29;
	string vjmkarvnyrotlur = "ywaozcblcmzolkykdaoyt";
	double bffxsxsppkdk = 8413;
	double tiezvu = 45773;
	int ufrzkgwcpli = 4872;
	return 97897;
}

string sdbfewb::yuwjkrdccjpiogvolynbhl(string zcywhho, bool amshyanxdna)
{
	double rkdifczuwhhqww = 23012;
	int lrsvz = 5511;
	if(23012 != 23012)
	{
		int qjkimm;
		for(qjkimm = 48; qjkimm > 0; qjkimm--)
		{
			continue;
		}
	}
	if(23012 == 23012)
	{
		int bwanbkkj;
		for(bwanbkkj = 64; bwanbkkj > 0; bwanbkkj--)
		{
			continue;
		}
	}
	if(5511 != 5511)
	{
		int ofcmo;
		for(ofcmo = 43; ofcmo > 0; ofcmo--)
		{
			continue;
		}
	}
	return string("urugewsbtpbel");
}

void sdbfewb::bhepanimtejfaqyeetrc(double kczpxloytyysb, double wenokmbhh, double dizxhmyht)
{
	int usabrrybrbyr = 4013;
	if(4013 == 4013)
	{
		int xsxmgfoy;
		for(xsxmgfoy = 65; xsxmgfoy > 0; xsxmgfoy--)
		{
			continue;
		}
	}
	if(4013 == 4013)
	{
		int gsdnbawz;
		for(gsdnbawz = 74; gsdnbawz > 0; gsdnbawz--)
		{
			continue;
		}
	}
	if(4013 == 4013)
	{
		int na;
		for(na = 28; na > 0; na--)
		{
			continue;
		}
	}
	if(4013 == 4013)
	{
		int rkmugiypqk;
		for(rkmugiypqk = 59; rkmugiypqk > 0; rkmugiypqk--)
		{
			continue;
		}
	}

}

sdbfewb::sdbfewb()
{
	this->hwtivjrruertnrjbtsbrriode(false, true, false, 14181, 59288, 0, string("iuwtogkewlezqzdgnikhryvgdonadqweqiwwgyezfdqajmvblydjxvftfstbexotkuzabszcwignejueyvuth"), 3431);
	this->yuwjkrdccjpiogvolynbhl(string("soubpxbbtthdaxvdblwamynxvdrmlndbldvgxybjayrymmjhhzmbfrqbhrcoahgpexrkwxixrddjtlxofrhbhslklkwkswjqip"), false);
	this->bhepanimtejfaqyeetrc(37015, 89619, 2739);
	this->nsqqlitniiepvytwijambd(false, 15892, 6410);
	this->loaugbetsn(string("nejqtrbjipfdswmtaamcmnvzzjuxosiwajxzgryzofjkbyjs"), 3645, true);
	this->pjpftfnobqdqukduzadyhl(5643, 5901);
	this->amcgrlmnquzgduotunwonjvra(string("jmamcrpyavywhpgtyzuufhwvcbeehbnixlwsdikiuwvchtzrdcfrvzpnjhcsynpyhuimcbgogbipiddmxwupcobkmfokisufixn"), true, 1723, 48710, 1028, 2356);
	this->kvdmsoaeicgpqjnsldnpuror(13088, 4140, 59083, 6148, 30286, 63574, 2516, 9655, 50635);
	this->folqpnhgblb(10169, false, 1855, false, string("mxhfvhewcuzzpxetuwzsyngajbizxjrhcugeriyzxpjvsdufunibtcngysdkcfnpg"));
	this->nybhhtjnpajbpmwftmhsjfbno(string(""), 842, true, 4151, 8567, string("mligxberudgsnpdbuhldinxoiboqgtmflwycmixqceuhxdrgviwewqyrgtokpnftoymwxkznyjctdif"), false, true, 6496, 2470);
	this->ajtdbdlodgwwjacpdepft(string("fnn"));
	this->pkoxvmqzypbhipdgd();
	this->kvppzlguapxawbmzcrxqcef(345, true, string("ywarssuflhysezhcgvihdlfwgzprgrfohmevahhpzbteobazfeucc"), 525, 15838, 2586, string("kuikziwnyrvboeihbwfnomemapsbevytnyuthmreaxxbrroetpreg"), false, 575, string("tirmkxxhmviubesmcarjyv"));
	this->fukvvzvaauhaoxzheprmu(string("qsrbjkckulewqfiwuyoxkmoxsenhozgnculouetdxcjcrvjpclakfdwgpjofjvgdhbkgfgup"), 4916, false, 59934);
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class qxjltok
{
public:
	bool pagdlinwsyos;
	bool oesogd;
	string bjggbfeueqxlh;
	double blffqkyhbv;
	qxjltok();
	double qsdueioaxewoivwrhctiobhjd(bool kaufhzktgwrmksb, int rjsfrhf, bool zcmxlmlybb, double yofuzqwqb, int fbaeq);
	double gnnrygpaecuhvbofp(bool tlyyyxlpobbno, double vjonxypcvvbwc, bool otxiydozgyskc, string jjdac, string ppaljpjgxipxi, int hukeanudb, string woxlbucgtey, int rugwr, int sndmwcifr);
	string ciltdbcpulnkoyo(bool xbdydstzudnq, string xcvchyo, int qkesjeszca, double upmjtzciopleh);
	int cmvvyvnpleicxyak(int tltrrr, string qajnekylpyfyw, string zajxgxrxtvqkguk, string czsjitagjwaepsg, double mxsmqte, double mfzucyvx, bool xhphpekprkcrkxi, double rbiniusxdzic);

protected:
	bool wprbhvdkf;
	int jkdowmggvem;

	string cqadqhkylopw(bool asfon, bool ymldxnbjac, int kujihjoudtzoo, string cwjbww, bool sovqugdyz, int umshnwruiiiz, int xsbwbcavwsc);
	int gselhrzejftz(string zmdumvl, double mlgbysdcn, int eiosucnpx, string aeqxpthrmkw, string kswgw);

private:
	bool ttuwmrjjiov;
	string lbtxu;

	bool fmyxhspmauv(int qcrlz, double sxqdibghbu);
	string jfkwxbidnvzmgxsxtuoh(bool lrjqhsx, string ratze, int owqrxcgbtmexj, bool lobeatnaynq, double girttn);
	void vgmhezanlr(int okavupv, double rjtjcyohobnznp, double gcohlktsyofe, double ceust, int zghtlhavtdlmo, bool jspuf, string nabcsn, bool hsejnuze, double nfsixv);
	bool dhlhaioabpamyeues(string nvtaxrob, bool tnxritqsf, double htrsiudisjodaqt, double ivtfwpypwj, double fsdzydkhzjcmqp, double jmfnpqqfql, string pzjsoldnygofxb, string dbyljbjorqzmii);
	int otnexycvkcbwzmh(string fsodffkanns, double vuxfzuswou, string fppmzcd, string jkutoq, int clbnhk);
	string knsykrwsbjb(bool xczjanycuaafl, bool cascca, string atpjjhdmiaa);
	int kepycykgxfadp();
	int tnghsevpuyxohbcklbn(bool symevy, string ljetebcxjsxijtv, int eyllidysffkyfvu, double ziwhepcaduyqp, int tedezunmtit, string zxfsluepzosor, bool zhhjcoozrh, string hnwvedmhcjbnd, string mhziodwziy, bool zhkzlmz);
	bool gxnqzoneejulrtzcpnyybr(string vdteigyoooj);
	string aqrwvvqxokfobtaf(int qcvlrojf, bool rzovsnkaapi);

};



bool qxjltok::fmyxhspmauv(int qcrlz, double sxqdibghbu)
{
	int gpzbypttcuydw = 730;
	double nojnes = 1713;
	int tutyj = 819;
	int eoaxnbcginsj = 3525;
	bool paikx = true;
	bool lkwgrloztjygsg = true;
	double mkibjnuxmncdti = 10713;
	bool omitxwtl = true;
	if(true == true)
	{
		int mpr;
		for(mpr = 46; mpr > 0; mpr--)
		{
			continue;
		}
	}
	if(730 != 730)
	{
		int uykbghrpu;
		for(uykbghrpu = 10; uykbghrpu > 0; uykbghrpu--)
		{
			continue;
		}
	}
	if(730 != 730)
	{
		int xuieklyot;
		for(xuieklyot = 14; xuieklyot > 0; xuieklyot--)
		{
			continue;
		}
	}
	if(730 == 730)
	{
		int uipzojn;
		for(uipzojn = 35; uipzojn > 0; uipzojn--)
		{
			continue;
		}
	}
	return false;
}

string qxjltok::jfkwxbidnvzmgxsxtuoh(bool lrjqhsx, string ratze, int owqrxcgbtmexj, bool lobeatnaynq, double girttn)
{
	int odrufd = 146;
	int uhpdasqwwq = 6721;
	bool msxxvyx = false;
	double ksqijgjus = 20530;
	if(20530 != 20530)
	{
		int ixelqban;
		for(ixelqban = 83; ixelqban > 0; ixelqban--)
		{
			continue;
		}
	}
	if(146 == 146)
	{
		int ptq;
		for(ptq = 95; ptq > 0; ptq--)
		{
			continue;
		}
	}
	if(146 == 146)
	{
		int zs;
		for(zs = 97; zs > 0; zs--)
		{
			continue;
		}
	}
	if(false != false)
	{
		int keekmm;
		for(keekmm = 92; keekmm > 0; keekmm--)
		{
			continue;
		}
	}
	return string("wjals");
}

void qxjltok::vgmhezanlr(int okavupv, double rjtjcyohobnznp, double gcohlktsyofe, double ceust, int zghtlhavtdlmo, bool jspuf, string nabcsn, bool hsejnuze, double nfsixv)
{
	double okoucdtf = 17185;
	if(17185 == 17185)
	{
		int skqm;
		for(skqm = 75; skqm > 0; skqm--)
		{
			continue;
		}
	}
	if(17185 != 17185)
	{
		int ss;
		for(ss = 26; ss > 0; ss--)
		{
			continue;
		}
	}
	if(17185 == 17185)
	{
		int asdtmklv;
		for(asdtmklv = 75; asdtmklv > 0; asdtmklv--)
		{
			continue;
		}
	}
	if(17185 == 17185)
	{
		int tpjohrx;
		for(tpjohrx = 9; tpjohrx > 0; tpjohrx--)
		{
			continue;
		}
	}
	if(17185 == 17185)
	{
		int nngf;
		for(nngf = 63; nngf > 0; nngf--)
		{
			continue;
		}
	}

}

bool qxjltok::dhlhaioabpamyeues(string nvtaxrob, bool tnxritqsf, double htrsiudisjodaqt, double ivtfwpypwj, double fsdzydkhzjcmqp, double jmfnpqqfql, string pzjsoldnygofxb, string dbyljbjorqzmii)
{
	int kfhuiim = 61;
	string swgekrdj = "rzyjtxcfplrxkyhghiygmg";
	int tzhpbeegfn = 1012;
	if(61 == 61)
	{
		int gq;
		for(gq = 93; gq > 0; gq--)
		{
			continue;
		}
	}
	if(string("rzyjtxcfplrxkyhghiygmg") != string("rzyjtxcfplrxkyhghiygmg"))
	{
		int qbjwlxhecm;
		for(qbjwlxhecm = 15; qbjwlxhecm > 0; qbjwlxhecm--)
		{
			continue;
		}
	}
	if(61 == 61)
	{
		int jfxwmldxjq;
		for(jfxwmldxjq = 34; jfxwmldxjq > 0; jfxwmldxjq--)
		{
			continue;
		}
	}
	if(1012 == 1012)
	{
		int usx;
		for(usx = 12; usx > 0; usx--)
		{
			continue;
		}
	}
	if(string("rzyjtxcfplrxkyhghiygmg") != string("rzyjtxcfplrxkyhghiygmg"))
	{
		int iojrfjd;
		for(iojrfjd = 76; iojrfjd > 0; iojrfjd--)
		{
			continue;
		}
	}
	return true;
}

int qxjltok::otnexycvkcbwzmh(string fsodffkanns, double vuxfzuswou, string fppmzcd, string jkutoq, int clbnhk)
{
	int emynstpmxltdd = 3334;
	int qgmzqxtth = 2236;
	double flssdogwvzfuqq = 52051;
	int mruzylwvtb = 3143;
	int vmedhpojqpsv = 1186;
	bool bgrudws = true;
	bool vmpjeakqy = true;
	bool gqvqq = false;
	bool gssifc = true;
	if(52051 != 52051)
	{
		int xtw;
		for(xtw = 99; xtw > 0; xtw--)
		{
			continue;
		}
	}
	return 19331;
}

string qxjltok::knsykrwsbjb(bool xczjanycuaafl, bool cascca, string atpjjhdmiaa)
{
	double ygpveznomjcut = 34985;
	string ugjqty = "xqtklheaewhaqggn";
	double mouskcjs = 451;
	string gdnwkuuuawcuef = "";
	int crcgst = 568;
	double acqqwrhreewahqy = 20189;
	bool kcqsatejqsqbk = true;
	if(20189 == 20189)
	{
		int ronkhunpf;
		for(ronkhunpf = 57; ronkhunpf > 0; ronkhunpf--)
		{
			continue;
		}
	}
	if(true != true)
	{
		int pg;
		for(pg = 68; pg > 0; pg--)
		{
			continue;
		}
	}
	if(true != true)
	{
		int tssef;
		for(tssef = 34; tssef > 0; tssef--)
		{
			continue;
		}
	}
	return string("atydufxhbnnmtwj");
}

int qxjltok::kepycykgxfadp()
{
	return 77569;
}

int qxjltok::tnghsevpuyxohbcklbn(bool symevy, string ljetebcxjsxijtv, int eyllidysffkyfvu, double ziwhepcaduyqp, int tedezunmtit, string zxfsluepzosor, bool zhhjcoozrh, string hnwvedmhcjbnd, string mhziodwziy, bool zhkzlmz)
{
	double hgqpq = 765;
	bool gnzdxrlxce = true;
	int hygplyltysumhc = 6096;
	double velazi = 34170;
	string ozvztdxvf = "fugumctomnulvzennknznkzqfsbgpbaueduwyhtzpezzzdhsiovgurxrkklalzksimucwukhuyvxisyyozzajwwkkfapempcyqtb";
	if(34170 != 34170)
	{
		int ydtodf;
		for(ydtodf = 26; ydtodf > 0; ydtodf--)
		{
			continue;
		}
	}
	if(34170 != 34170)
	{
		int rqtmjxx;
		for(rqtmjxx = 64; rqtmjxx > 0; rqtmjxx--)
		{
			continue;
		}
	}
	if(765 == 765)
	{
		int lilciglwf;
		for(lilciglwf = 47; lilciglwf > 0; lilciglwf--)
		{
			continue;
		}
	}
	if(34170 != 34170)
	{
		int nn;
		for(nn = 61; nn > 0; nn--)
		{
			continue;
		}
	}
	if(true != true)
	{
		int lpuh;
		for(lpuh = 53; lpuh > 0; lpuh--)
		{
			continue;
		}
	}
	return 36690;
}

bool qxjltok::gxnqzoneejulrtzcpnyybr(string vdteigyoooj)
{
	double cmujwtqovetmd = 38708;
	string jjnjfulitk = "mqiotnfhgqybekkohrhvhbxaafhefytmlvnkernkpofwgbfvofqmafwdybaaskmgfshfqfjy";
	int lnxvyacqomv = 8684;
	int byuilmn = 6263;
	bool xalubw = true;
	bool cxiwsbua = false;
	int qmguu = 200;
	if(false == false)
	{
		int ioxxwt;
		for(ioxxwt = 32; ioxxwt > 0; ioxxwt--)
		{
			continue;
		}
	}
	if(false == false)
	{
		int ywbmxdaky;
		for(ywbmxdaky = 1; ywbmxdaky > 0; ywbmxdaky--)
		{
			continue;
		}
	}
	if(false == false)
	{
		int mozohzwfcp;
		for(mozohzwfcp = 7; mozohzwfcp > 0; mozohzwfcp--)
		{
			continue;
		}
	}
	if(true != true)
	{
		int ogmnfthamp;
		for(ogmnfthamp = 55; ogmnfthamp > 0; ogmnfthamp--)
		{
			continue;
		}
	}
	if(6263 != 6263)
	{
		int uxaovhpb;
		for(uxaovhpb = 2; uxaovhpb > 0; uxaovhpb--)
		{
			continue;
		}
	}
	return true;
}

string qxjltok::aqrwvvqxokfobtaf(int qcvlrojf, bool rzovsnkaapi)
{
	int qnuejutydcesmng = 2175;
	string dtucm = "bnsvqqvvvzeqhjgs";
	string eqgvozlje = "ngcskzpfhhjweoxjbs";
	string wlgeatcyoo = "kksxajkileiyqferwjyfbrjpjewvjehnogoivdme";
	double gkprbrdxoeby = 55020;
	int jzbvqre = 4841;
	if(2175 == 2175)
	{
		int whrft;
		for(whrft = 48; whrft > 0; whrft--)
		{
			continue;
		}
	}
	if(string("ngcskzpfhhjweoxjbs") != string("ngcskzpfhhjweoxjbs"))
	{
		int witwevn;
		for(witwevn = 6; witwevn > 0; witwevn--)
		{
			continue;
		}
	}
	if(string("ngcskzpfhhjweoxjbs") != string("ngcskzpfhhjweoxjbs"))
	{
		int vad;
		for(vad = 99; vad > 0; vad--)
		{
			continue;
		}
	}
	return string("nvy");
}

string qxjltok::cqadqhkylopw(bool asfon, bool ymldxnbjac, int kujihjoudtzoo, string cwjbww, bool sovqugdyz, int umshnwruiiiz, int xsbwbcavwsc)
{
	string ojffphkffod = "ufyvfnxhjbllnwktlrtuybftotyfnhqjjluyukslnuropmzwwsypyqwohxfktstwioydwiplswhbbv";
	int ivycdngdks = 4210;
	double qaxmbfjkrzio = 71020;
	int kdvvkv = 3398;
	int jzrwr = 1184;
	double qqreralbyz = 4584;
	int eenordqyegeewa = 5397;
	bool wbywldlujzusp = false;
	if(3398 != 3398)
	{
		int bjvdiicsy;
		for(bjvdiicsy = 69; bjvdiicsy > 0; bjvdiicsy--)
		{
			continue;
		}
	}
	if(4584 != 4584)
	{
		int sgeiu;
		for(sgeiu = 15; sgeiu > 0; sgeiu--)
		{
			continue;
		}
	}
	return string("acyosmvwujn");
}

int qxjltok::gselhrzejftz(string zmdumvl, double mlgbysdcn, int eiosucnpx, string aeqxpthrmkw, string kswgw)
{
	int aiclzzrrh = 5200;
	double pvlhlmtggth = 5837;
	int riifgopmowcwd = 3962;
	bool jfpyxfmfrq = false;
	double ekmufaogzpck = 32174;
	int zuogov = 8968;
	if(5200 == 5200)
	{
		int wl;
		for(wl = 99; wl > 0; wl--)
		{
			continue;
		}
	}
	if(8968 != 8968)
	{
		int kaulukemm;
		for(kaulukemm = 71; kaulukemm > 0; kaulukemm--)
		{
			continue;
		}
	}
	if(5837 == 5837)
	{
		int ufswma;
		for(ufswma = 62; ufswma > 0; ufswma--)
		{
			continue;
		}
	}
	if(3962 == 3962)
	{
		int zga;
		for(zga = 3; zga > 0; zga--)
		{
			continue;
		}
	}
	if(3962 != 3962)
	{
		int pt;
		for(pt = 39; pt > 0; pt--)
		{
			continue;
		}
	}
	return 22254;
}

double qxjltok::qsdueioaxewoivwrhctiobhjd(bool kaufhzktgwrmksb, int rjsfrhf, bool zcmxlmlybb, double yofuzqwqb, int fbaeq)
{
	bool migymjumqilptrt = false;
	bool jaaswww = true;
	bool seznelhnstwpdlb = true;
	bool njfrj = false;
	string ixuokyipeqqj = "weinrrdgiwhhrtauzstltvrqjbqoazstobmcnppryykytdqcmbfjixnmjgxbcrxefimtain";
	string gniqphpmpthfyu = "mnojewymsplmdkqambfbgxywherzylshmmhziacpyjfstwgcutcteiczcyajfejvcrligujxelvltlwgjqjzankhdtzsau";
	bool kpasjuvzfyb = true;
	int ctbkbvopjzl = 6792;
	if(true != true)
	{
		int jimhd;
		for(jimhd = 62; jimhd > 0; jimhd--)
		{
			continue;
		}
	}
	if(string("mnojewymsplmdkqambfbgxywherzylshmmhziacpyjfstwgcutcteiczcyajfejvcrligujxelvltlwgjqjzankhdtzsau") != string("mnojewymsplmdkqambfbgxywherzylshmmhziacpyjfstwgcutcteiczcyajfejvcrligujxelvltlwgjqjzankhdtzsau"))
	{
		int mvmvc;
		for(mvmvc = 34; mvmvc > 0; mvmvc--)
		{
			continue;
		}
	}
	if(true != true)
	{
		int ceedelfu;
		for(ceedelfu = 20; ceedelfu > 0; ceedelfu--)
		{
			continue;
		}
	}
	if(false != false)
	{
		int jy;
		for(jy = 30; jy > 0; jy--)
		{
			continue;
		}
	}
	if(true != true)
	{
		int pizl;
		for(pizl = 32; pizl > 0; pizl--)
		{
			continue;
		}
	}
	return 77155;
}

double qxjltok::gnnrygpaecuhvbofp(bool tlyyyxlpobbno, double vjonxypcvvbwc, bool otxiydozgyskc, string jjdac, string ppaljpjgxipxi, int hukeanudb, string woxlbucgtey, int rugwr, int sndmwcifr)
{
	double psvywcahvzzy = 1387;
	return 95686;
}

string qxjltok::ciltdbcpulnkoyo(bool xbdydstzudnq, string xcvchyo, int qkesjeszca, double upmjtzciopleh)
{
	string ukspwrosagq = "oipgujiktaanxywwwelgvwbhhweuurnpyrysodnipdhgqyunnkhjmufkhvvjzeowpcgcgafdxautkwjpofma";
	if(string("oipgujiktaanxywwwelgvwbhhweuurnpyrysodnipdhgqyunnkhjmufkhvvjzeowpcgcgafdxautkwjpofma") == string("oipgujiktaanxywwwelgvwbhhweuurnpyrysodnipdhgqyunnkhjmufkhvvjzeowpcgcgafdxautkwjpofma"))
	{
		int zapf;
		for(zapf = 8; zapf > 0; zapf--)
		{
			continue;
		}
	}
	return string("njpqbfktncgdkn");
}

int qxjltok::cmvvyvnpleicxyak(int tltrrr, string qajnekylpyfyw, string zajxgxrxtvqkguk, string czsjitagjwaepsg, double mxsmqte, double mfzucyvx, bool xhphpekprkcrkxi, double rbiniusxdzic)
{
	bool anqdnkndable = true;
	string xdkawjrfvz = "ihsdydsivkxqsxakczlmhkrlirqgttesctewdaptsebosymxxzsjvxolvanunripducxjfzcbmppivkc";
	bool okqyxs = false;
	string lwhzisxkfvdv = "ybynbxootueboeafxofoqmlkdywvqywpbdbpy";
	bool qahvvavewt = false;
	double jfdqsqqketaujrf = 22759;
	bool alwnlc = false;
	double ngkbbgxlyrtigw = 18497;
	int wzfmrexzxx = 400;
	if(18497 == 18497)
	{
		int kgwkv;
		for(kgwkv = 8; kgwkv > 0; kgwkv--)
		{
			continue;
		}
	}
	if(string("ybynbxootueboeafxofoqmlkdywvqywpbdbpy") == string("ybynbxootueboeafxofoqmlkdywvqywpbdbpy"))
	{
		int bbrjcsaknt;
		for(bbrjcsaknt = 61; bbrjcsaknt > 0; bbrjcsaknt--)
		{
			continue;
		}
	}
	if(18497 == 18497)
	{
		int px;
		for(px = 2; px > 0; px--)
		{
			continue;
		}
	}
	if(false == false)
	{
		int kjy;
		for(kjy = 44; kjy > 0; kjy--)
		{
			continue;
		}
	}
	if(string("ybynbxootueboeafxofoqmlkdywvqywpbdbpy") == string("ybynbxootueboeafxofoqmlkdywvqywpbdbpy"))
	{
		int ytkwmouikw;
		for(ytkwmouikw = 69; ytkwmouikw > 0; ytkwmouikw--)
		{
			continue;
		}
	}
	return 9327;
}

qxjltok::qxjltok()
{
	this->qsdueioaxewoivwrhctiobhjd(true, 775, true, 69663, 2394);
	this->gnnrygpaecuhvbofp(false, 22718, true, string("tohgqekwcweoyklwflilotggpjwktimnouqryenzsjwympdmbekbjimwjedolclyjvoucawhjkvjptuprqusalicvqnolhmurs"), string("bcipkvgknfycmapuodjcvivhcehxnfkikwsxidtjkrqiyok"), 2404, string("fwuhkjrkhdncemaemwueqiccrdquvrubmpwcjfwjbbdphhhebpqhojyuzyxgyyfktbirxumlhjkdwsopjlgpxsibtxnynpkcujms"), 1102, 2297);
	this->ciltdbcpulnkoyo(true, string("uyjohxipjrgtreyeqwgdrxozzivatanlpcliqajsvgznxyhd"), 4118, 23333);
	this->cmvvyvnpleicxyak(342, string("uvgiehenzyxidiiemoicuskyscvv"), string("hlgfiwgtpxeizwfhzbzlzpajxivrjjnckvkkkiuutyfwzldiefncondtqchmsdcqkxpdtmzqicwdpus"), string("vzfvzxwyykezcovp"), 33012, 60995, false, 16079);
	this->cqadqhkylopw(false, true, 1713, string("fyqshfednxat"), false, 4375, 618);
	this->gselhrzejftz(string("leiihbxzzbsjwqalgkououtwncohvmsxmpuveifrirmazyomvmunixauwwkrfduaysdbkjxaxucjfj"), 27329, 6, string("nswirtcrllnvchlvtkhygnkr"), string("qprerutthqfzosjyeutnfuuopyelwkwlvrufgccuaprixzlbwmzyaooekhwpey"));
	this->fmyxhspmauv(4615, 24869);
	this->jfkwxbidnvzmgxsxtuoh(false, string("uwutacheoziapqhjornsbjqnzihplgwywwvyeykwtshfglcqxzhryuxuwjcxtalxjqnjba"), 5726, false, 17843);
	this->vgmhezanlr(704, 8032, 37839, 5990, 2283, false, string("phebzzvrcbqonnycqsrqndgapwbeeujlsmnomgauziucrsaomehjmrbbvlujhruehegxadwgmriaxokoqqgabemlvhiktffudgtc"), true, 33600);
	this->dhlhaioabpamyeues(string("hdnghuncwcbwmczmzcdiumxccovmrmawzrsefoirbrqzxozovejkzfqbbhek"), false, 4571, 58420, 55881, 3410, string("ujlfoevsuhqkxsydplrlwsvwupybpprnbrsbhyslqekrx"), string("babwtfsrvoxkevqwalfwmlnoklwc"));
	this->otnexycvkcbwzmh(string("yzdvilklxcgiybxlygiacroxructlrdlqtodwydypmyszbizmlqwvijafkhglhmjpxeuprr"), 22046, string("sdmacwxqogpcqqzsvqcbmrfijizmbdbeiedgavkenhpvozhrsphbuccjojvlpbqenrevehgsgpzev"), string("xiunnnypuyuvfvxkonagufkjldwilvgrw"), 5692);
	this->knsykrwsbjb(false, true, string("asvrqagkizvfmywttufzgnipmwpllhcyigjmihpbujrmebepwixpxjaxzrnonydvm"));
	this->kepycykgxfadp();
	this->tnghsevpuyxohbcklbn(true, string("zuxpedegupbpteqwvcyd"), 1112, 31256, 1641, string("amafbgjcwbnoxadzkwuitqrycxteqptzmyjirmnsplcavkqyosqxavjetrumfnwndsnrvjvobbyvxxuevazbeusgsrdvmcbi"), true, string("rjqxqoqwufztsrzxzovlmwmvcghpsiiortg"), string("qxsrtjoexhtdykqvffwsexhoertarqnnqvlkjsdnudwkkisdnkfstavbthylprwtidogpjtzervizpivwj"), false);
	this->gxnqzoneejulrtzcpnyybr(string("orggkwzepkgng"));
	this->aqrwvvqxokfobtaf(9022, false);
}