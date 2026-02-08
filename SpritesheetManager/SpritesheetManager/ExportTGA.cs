using System;
using System.Collections.Generic;
using Gtk;
using System.IO;
using Newtonsoft.Json;

namespace SpritesheetManager
{
    public class ExportData
    {
        public Gdk.Rectangle rect { get; set; }
        public int id { get; set; }
    }

    public class ExportTGA
    {
        static string outputpath;

        public static bool ExportToTGA(List<MetaData> data)
        {
            if (data == null || data.Count < 1)
            {
                new MessageDialog(
                    MainClass.win,
                    DialogFlags.Modal,
                    MessageType.Info,
                    ButtonsType.Ok,
                    "You cant save nothing."
                ).Run();
                return false;
            }

            FileChooserDialog fileDialog = new FileChooserDialog(
                "Choose a file",
                MainClass.win,
                FileChooserAction.Save,
                "Cancel", ResponseType.Cancel,
                "Save", ResponseType.Accept
            );

            FileFilter filter = new FileFilter();
            filter.Name = "spritesheet format | *.png";
            filter.AddPattern("*.png");
            fileDialog.AddFilter(filter);

            string outputpath = "";
            if (fileDialog.Run() == (int)ResponseType.Accept)
            {
                outputpath = fileDialog.Filename;
            }
            else
            {
                fileDialog.Dispose();
                return false;
            }

            List<Gdk.Pixbuf> validPixbufs = new List<Gdk.Pixbuf>();
            List<MetaData> validMeta = new List<MetaData>();
            int totalwidth = 0;
            int maxheight = 0;

            foreach (var meta in data)
            {
                try
                {
                    string fullPath = Path.GetFullPath(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, meta.relativepath));

                    Console.WriteLine(fullPath);

                    if (!File.Exists(fullPath))
                    { 
                        Console.WriteLine($"invalid file path");
                        continue;
                    }

                    using (var pix = new Gdk.Pixbuf(fullPath))
                    {
                        if (pix.Width <= 0 || pix.Height <= 0) continue;

                        validPixbufs.Add(pix.Copy());
                        validMeta.Add(meta);
                        totalwidth += pix.Width;
                        if (pix.Height > maxheight) maxheight = pix.Height;
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"{ex.Message}");
                }
            }

            if (validPixbufs.Count == 0)
            {
                new MessageDialog(
                    MainClass.win,
                    DialogFlags.Modal,
                    MessageType.Error,
                    ButtonsType.Ok,
                    "No valid images to export."
                ).Run();
                fileDialog.Dispose();
                return false;
            }

            using (var atlas = new Gdk.Pixbuf(Gdk.Colorspace.Rgb, true, 8, totalwidth, maxheight))
            {
                atlas.Fill(0x00000000);
                int offsetx = 0;
                List<ExportData> exportdata = new List<ExportData>();

                for (int i = 0; i < validPixbufs.Count; i++)
                {
                    var pix = validPixbufs[i];
                    var meta = validMeta[i];

                    pix.CopyArea(0, 0, pix.Width, pix.Height, atlas, offsetx, 0);

                    meta.rect = new Gdk.Rectangle(offsetx, 0, pix.Width, pix.Height);

                    exportdata.Add(new ExportData
                    {
                        rect = new Gdk.Rectangle(offsetx, 0, pix.Width, pix.Height),
                        id = i
                    });

                    offsetx += pix.Width;
                    pix.Dispose();
                }

                atlas.Save(outputpath, "png");
                File.WriteAllText(outputpath + ".atlas", JsonConvert.SerializeObject(exportdata, Newtonsoft.Json.Formatting.Indented));
            }

            fileDialog.Dispose();
            return true;
        }

    }
}
