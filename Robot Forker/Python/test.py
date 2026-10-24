import wx

class Gui(wx.Frame):

    def __init__(self, title):
        """Initialise widgets and layout."""
        super().__init__(parent=None, title=title, size=(400, 400))

        # Configure the file menu
        fileMenu = wx.Menu()
        menuBar = wx.MenuBar()
        fileMenu.Append(wx.ID_EXIT, "&Exit")
        menuBar.Append(fileMenu, "&File")
        self.SetMenuBar(menuBar)

        # Configure the widgets
        self.text = wx.StaticText(self, wx.ID_ANY, "Some text")
        self.button1 = wx.Button(self, wx.ID_ANY, "Button1")
        # Bind events to widgets
        self.Bind(wx.EVT_MENU, self.OnMenu)
        self.button1.Bind(wx.EVT_BUTTON, self.OnButton1)
        # Configure sizers for layout
        main_sizer = wx.BoxSizer(wx.HORIZONTAL)
        side_sizer = wx.BoxSizer(wx.VERTICAL)
        main_sizer.Add(side_sizer, 1, wx.ALL, 5)

        side_sizer.Add(self.text, 1, wx.TOP, 10)
        side_sizer.Add(self.button1, 1, wx.ALL, 5)

        self.SetSizeHints(300, 300)
        self.SetSizer(main_sizer)

    def OnMenu(self, event):
        """Handle the event when the user selects a menu item."""
        Id = event.GetId()
        if Id == wx.ID_EXIT:
            self.Close(True)
 
    def OnButton1(self, event):
        """Handle the event when the user clicks button1."""
        print ("Button 1 pressed")

app = wx.App()
gui = Gui("Demo")
gui.Show(True)
app.MainLoop()