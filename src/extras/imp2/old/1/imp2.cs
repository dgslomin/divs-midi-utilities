
namespace Sreal.Imp2
{
	class Application
	{
		static void Main(string[] args)
		{
			System.Windows.Forms.Application.Run(new Sreal.Imp2.MainForm());
		}
	}

	class MainForm: System.Windows.Forms.Form
	{
		public MainForm()
		{
			this.Text = "Imp";
			this.BackgroundImage = System.Drawing.Image.FromStream(System.Reflection.Assembly.GetExecutingAssembly().GetManifestResourceStream("keyboard.png"));
			this.ClientSize = this.BackgroundImage.Size;
		}
	}
}

