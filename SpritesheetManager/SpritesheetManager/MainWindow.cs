using System;
using Gtk;
using SpritesheetManager;
using System.IO;
using System.Text;
using Gdk;

public partial class MainWindow : Gtk.Window
{
    public SpriteType currenttype;
    public RadioButton selectedpreset;
    public VBox rowsContainer;
    public static int highesint = 0;

    public static string RelativePath(string absPath, string relTo, bool inclfile = true)//https://iandevlin.com/blog/2010/01/csharp/generating-a-relative-path-in-csharp/
    {
        string[] absDirs = absPath.TrimEnd('/').Split('/');
        string[] relDirs = relTo.TrimEnd('/').Split('/');

        int lastCommonRoot = -1;
        int len = Math.Min(absDirs.Length, relDirs.Length);
        for (int i = 0; i < len; i++)
        {
            if (absDirs[i] == relDirs[i]) lastCommonRoot = i;
            else break;
        }

        if (lastCommonRoot == -1)
            throw new ArgumentException("Paths do not have a common base");

        StringBuilder relativePath = new StringBuilder();

        for (int i = lastCommonRoot + 1; i < absDirs.Length - (inclfile ? 1 : 0); i++)
        {
            relativePath.Append("../");
        }

        for (int i = lastCommonRoot + 1; i < relDirs.Length; i++)
        {
            relativePath.Append(relDirs[i]);
            if (i < relDirs.Length - 1) relativePath.Append("/");
        }

        Console.WriteLine(relativePath.ToString());

        return relativePath.ToString();
    }

    public void ClearAllNow()
    {
        SaveData.Coredata.Clear();
        foreach (var a in scrolledwindow2.Children)
        {
            scrolledwindow2.Remove(a);
            a.Destroy();
        }
        highesint = 0;
        return;
    }

    public void UpdateType()
    {
        if (animation.Active) { currenttype = SpriteType.Animated; currentlysel.Text = "currently selected :\nAnimation"; selectedpreset = animation; }
        else if (cubemap.Active) { currenttype = SpriteType.Cubemap; currentlysel.Text = "currently selected :\nCubemap"; selectedpreset = cubemap; }
        else if (spritesheet.Active) { currenttype = SpriteType.Spritesheet; currentlysel.Text = "currently selected :\nSpritesheet"; selectedpreset = spritesheet; }
        return;
    }

    public MainWindow() : base(Gtk.WindowType.Toplevel)
    {
        Build();

        UpdateType();

        rowsContainer = new VBox();
        rowsContainer.Homogeneous = false;
        rowsContainer.Spacing = 2;

        rowsContainer.SetSizeRequest(-1, 1);

        Viewport viewport = new Viewport();
        viewport.Add(rowsContainer);
        scrolledwindow2.Add(viewport);

        rowsContainer.ShowAll();
        viewport.ShowAll();
    }

    protected void OnDeleteEvent(object sender, DeleteEventArgs a)
    {
        Application.Quit();
        a.RetVal = true;
    }

    protected void OnInfoClicked(object sender, EventArgs e)
    {
    }

    protected void OnCleardataClicked(object sender, EventArgs e)
    {
        MessageDialog md = new MessageDialog(
            MainClass.win,
            DialogFlags.Modal,
            MessageType.Question,
            ButtonsType.YesNo,
            "Are you sure you want to continue?\nthis will wipe all your unsaved progress."
            );

        ResponseType response = (ResponseType)md.Run();

        md.Destroy();

        if (response == ResponseType.No)
        {
            return;
        }

        ClearAllNow();
    }

    protected void OnAddelementClicked(object sender, EventArgs e)
    {
        FileChooserDialog fileDialog = new FileChooserDialog(
            "Select an image",
            MainClass.win,
            FileChooserAction.Open,
            "Cancel", ResponseType.Cancel,
            "Open", ResponseType.Accept
        );

        FileFilter filter = new FileFilter();
        filter.Name = "Image files | *.png, *.jpg, *.jpeg, *.bmp";
        filter.AddPattern("*.png");
        filter.AddPattern("*.jpg");
        filter.AddPattern("*.jpeg");
        filter.AddPattern("*.bmp");
        fileDialog.AddFilter(filter);

        string selectedPath = null;
        if (fileDialog.Run() == (int)ResponseType.Accept)
        {
            try
            {
                selectedPath = RelativePath(AppDomain.CurrentDomain.BaseDirectory, fileDialog.Filename);
            }
            catch (Exception ex)
            {
                MessageDialog md = new MessageDialog(
                    MainClass.win,
                    DialogFlags.Modal,
                    MessageType.Error,
                    ButtonsType.Ok,
                    ex.Message
                );
                md.Run();
                md.Destroy();
                fileDialog.Destroy();
                return;
            }
        }
        else
        {
            fileDialog.Destroy();
            return;
        }

        Pixbuf img;

        try
        {
            img = new Pixbuf(fileDialog.Filename);
        }
        catch
        {
            MessageDialog md = new MessageDialog(
                MainClass.win,
                DialogFlags.Modal,
                MessageType.Warning,
                ButtonsType.Ok,
                "This image seems to be corrupt.\nor the path is invalid"
            );
            md.Run();
            md.Destroy();
            fileDialog.Destroy();
            return;
        }

        MetaData index = new MetaData
        {
            id = highesint,
            relativepath = selectedPath,
            rect = new Rectangle(0, 0, img.Width, img.Height),
            spritetype = currenttype,
            frame = 0
        };

        highesint = highesint + 1;

        SaveData.Coredata.Add(index);
        img.Dispose();

        HBox row = new HBox();
        row.Spacing = 5;

        Label idLabel = new Label($"{index.id}");
        idLabel.TooltipText = "ID element";
        row.PackStart(idLabel, false, false, 5);

        Label pathLabel = new Label(index.relativepath.Substring(0,32));
        row.PackStart(pathLabel, false, false, 5);

        Entry rectXEntry = new Entry(index.rect.X.ToString());
        rectXEntry.TooltipText = "rectangle X cord";
        rectXEntry.WidthChars = 3;
        rectXEntry.Changed += (s, ev) =>
        {
            if (int.TryParse(rectXEntry.Text, out int x))
                index.rect = new Rectangle(x, index.rect.Y, index.rect.Width, index.rect.Height);
        };
        row.PackStart(rectXEntry, false, false, 2);

        Entry rectYEntry = new Entry(index.rect.Y.ToString());
        rectYEntry.TooltipText = "rectangle Y cord";
        rectYEntry.WidthChars = 3;
        rectYEntry.Changed += (s, ev) =>
        {
            if (int.TryParse(rectYEntry.Text, out int y))
                index.rect = new Rectangle(index.rect.X, y, index.rect.Width, index.rect.Height);
        };
        row.PackStart(rectYEntry, false, false, 2);

        Entry rectWEntry = new Entry(index.rect.Width.ToString());
        rectWEntry.TooltipText = "rectangle W cord";
        rectWEntry.WidthChars = 3;
        rectWEntry.Changed += (s, ev) =>
        {
            if (int.TryParse(rectWEntry.Text, out int w))
                index.rect = new Rectangle(index.rect.X, index.rect.Y, w, index.rect.Height);
        };
        row.PackStart(rectWEntry, false, false, 2);

        Entry rectHEntry = new Entry(index.rect.Height.ToString());
        rectHEntry.TooltipText = "rectangle H cord";
        rectHEntry.WidthChars = 3;
        rectHEntry.Changed += (s, ev) =>
        {
            if (int.TryParse(rectHEntry.Text, out int h))
                index.rect = new Rectangle(index.rect.X, index.rect.Y, index.rect.Width, h);
        };
        row.PackStart(rectHEntry, false, false, 2);

        Entry frameEntry = new Entry(index.frame.ToString());
        frameEntry.TooltipText = "frame (animation) / face (cubemap) id";
        frameEntry.WidthChars = 2;
        frameEntry.Changed += (s, ev) =>
        {
            if (int.TryParse(frameEntry.Text, out int f))
                index.frame = f;
        };
        row.PackStart(frameEntry, false, false, 2);

        Button delBtn = new Button("X");
        delBtn.Clicked += (s, ev) =>
        {
            SaveData.Coredata.Remove(index);
            rowsContainer.Remove(row);
            row.Destroy();

            for (int i = 0; i < SaveData.Coredata.Count; i++)
            {
                SaveData.Coredata[i].id = i;
                HBox r = (HBox)rowsContainer.Children[i];
                Label idLbl = (Label)r.Children[0];
                idLbl.Text = i.ToString();
            }
        };
        row.PackStart(delBtn, false, false, 5);

        rowsContainer.PackStart(row, false, false, 2);
        rowsContainer.ShowAll();

        fileDialog.Destroy();
    }

    protected void OnSaveClicked(object sender, EventArgs e)
    {
        SaveData.SaveDataNow();
    }

    protected void OnLoadClicked(object sender, EventArgs e)
    {
        SaveData.LoadData();
    }

    protected void OnSpritesheetToggled(object sender, EventArgs e)
    {
        RadioButton rb = sender as RadioButton;

        if (!rb.Active)
            return;

        MessageDialog md = new MessageDialog(
            MainClass.win,
            DialogFlags.Modal,
            MessageType.Question,
            ButtonsType.YesNo,
            "Are you sure you want to switch preset?\nthis will wipe all your unsaved progress."
            );

        ResponseType response = (ResponseType)md.Run();

        md.Destroy();

        if (response == ResponseType.No)
        {
            selectedpreset.Active = true;
            return;
        }
        ClearAllNow();
        UpdateType();
    }

    protected void OnCubemapToggled(object sender, EventArgs e)
    {
        RadioButton rb = sender as RadioButton;

        if (!rb.Active)
            return;

        MessageDialog md = new MessageDialog(
            MainClass.win,
            DialogFlags.Modal,
            MessageType.Question,
            ButtonsType.YesNo,
            "Are you sure you want to switch preset?\nthis will wipe all your unsaved progress."
            );

        ResponseType response = (ResponseType)md.Run();

        md.Destroy();

        if (response == ResponseType.No)
        {
            selectedpreset.Active = true;
            return;
        }
        ClearAllNow();
        UpdateType();
    }

    protected void OnAnimationToggled(object sender, EventArgs e)
    {
        RadioButton rb = sender as RadioButton;

        if (!rb.Active)
            return;

        MessageDialog md = new MessageDialog(
            MainClass.win,
            DialogFlags.Modal,
            MessageType.Question,
            ButtonsType.YesNo,
            "Are you sure you want to switch preset?\nthis will wipe all your unsaved progress."
        );

        ResponseType response = (ResponseType)md.Run();

        md.Destroy();

        if (response == ResponseType.No)
        {
            selectedpreset.Active = true;
            return;
        }
        ClearAllNow();
        UpdateType();
    }

    protected void OnExporttgaClicked(object sender, EventArgs e)
    {
        ExportTGA.ExportToTGA(SaveData.Coredata);
    }
}
