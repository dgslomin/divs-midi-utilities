
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
		System.Windows.Forms.TreeView TreeView;
		System.Windows.Forms.RadioButton UnassignedRadioButton;
		System.Windows.Forms.RadioButton NoteRadioButton;
		System.Windows.Forms.NumericUpDown NoteNumericUpDown;
		System.Windows.Forms.ComboBox NoteChannelComboBox;
		System.Windows.Forms.RadioButton TransposeRadioButton;
		System.Windows.Forms.NumericUpDown TransposeNumericUpDown;
		System.Windows.Forms.CheckBox TransposeMomentaryCheckBox;
		System.Windows.Forms.CheckBox TransposeAbsoluteCheckBox;
		System.Windows.Forms.RadioButton VelocityRadioButton;
		System.Windows.Forms.NumericUpDown VelocityNumericUpDown;
		System.Windows.Forms.CheckBox VelocityMomentaryCheckBox;
		System.Windows.Forms.CheckBox VelocityAbsoluteCheckBox;
		System.Windows.Forms.RadioButton PanicRadioButton;
		System.Windows.Forms.RichTextBox InfoRichTextBox;

		public MainForm()
		{
			this.Text = "Imp";
			this.Size = new System.Drawing.Size(640, 480);

			System.Windows.Forms.SplitContainer SplitContainer = new System.Windows.Forms.SplitContainer();
			SplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
			this.Controls.Add(SplitContainer);

			TreeView = new System.Windows.Forms.TreeView();
			TreeView.Dock = System.Windows.Forms.DockStyle.Fill;
			SplitContainer.Panel1.Controls.Add(TreeView);

			System.Windows.Forms.FlowLayoutPanel FlowLayoutPanel = new System.Windows.Forms.FlowLayoutPanel();
			FlowLayoutPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			FlowLayoutPanel.Padding = new System.Windows.Forms.Padding(4, 8, 4, 4);
			FlowLayoutPanel.Dock = System.Windows.Forms.DockStyle.Fill;
			SplitContainer.Panel2.Controls.Add(FlowLayoutPanel);

			System.Windows.Forms.Label KeyMappingLabel = new System.Windows.Forms.Label();
			KeyMappingLabel.Text = "Key Mapping";
			KeyMappingLabel.Font = new System.Drawing.Font(KeyMappingLabel.Font.FontFamily, 14, System.Drawing.FontStyle.Bold);
			KeyMappingLabel.AutoSize = true;
			KeyMappingLabel.Dock = System.Windows.Forms.DockStyle.Top | System.Windows.Forms.DockStyle.Bottom;
			FlowLayoutPanel.Controls.Add(KeyMappingLabel);
			FlowLayoutPanel.SetFlowBreak(KeyMappingLabel, true);

			UnassignedRadioButton = new System.Windows.Forms.RadioButton();
			UnassignedRadioButton.Text = "Unassigned";
			UnassignedRadioButton.Checked = true;
			UnassignedRadioButton.AutoSize = true;
			UnassignedRadioButton.Dock = System.Windows.Forms.DockStyle.Top | System.Windows.Forms.DockStyle.Bottom;
			FlowLayoutPanel.Controls.Add(UnassignedRadioButton);
			FlowLayoutPanel.SetFlowBreak(UnassignedRadioButton, true);

			NoteRadioButton = new System.Windows.Forms.RadioButton();
			NoteRadioButton.Text = "Note";
			NoteRadioButton.AutoSize = true;
			NoteRadioButton.Dock = System.Windows.Forms.DockStyle.Top | System.Windows.Forms.DockStyle.Bottom;
			FlowLayoutPanel.Controls.Add(NoteRadioButton);
			FlowLayoutPanel.SetFlowBreak(NoteRadioButton, false);

			NoteNumericUpDown = new System.Windows.Forms.NumericUpDown();
			NoteNumericUpDown.AutoSize = true;
			NoteNumericUpDown.Minimum = 1;
			NoteNumericUpDown.Maximum = 128;
			NoteNumericUpDown.Value = 60;
			NoteNumericUpDown.Dock = System.Windows.Forms.DockStyle.Top | System.Windows.Forms.DockStyle.Bottom;
			FlowLayoutPanel.Controls.Add(NoteNumericUpDown);
			FlowLayoutPanel.SetFlowBreak(NoteNumericUpDown, false);

			NoteChannelComboBox = new System.Windows.Forms.ComboBox();
			NoteChannelComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			NoteChannelComboBox.Items.Add("Default Channel");
			NoteChannelComboBox.Items.Add("Channel 1");
			NoteChannelComboBox.Items.Add("Channel 2");
			NoteChannelComboBox.Items.Add("Channel 3");
			NoteChannelComboBox.Items.Add("Channel 4");
			NoteChannelComboBox.Items.Add("Channel 5");
			NoteChannelComboBox.Items.Add("Channel 6");
			NoteChannelComboBox.Items.Add("Channel 7");
			NoteChannelComboBox.Items.Add("Channel 8");
			NoteChannelComboBox.Items.Add("Channel 9");
			NoteChannelComboBox.Items.Add("Channel 10");
			NoteChannelComboBox.Items.Add("Channel 11");
			NoteChannelComboBox.Items.Add("Channel 12");
			NoteChannelComboBox.Items.Add("Channel 13");
			NoteChannelComboBox.Items.Add("Channel 14");
			NoteChannelComboBox.Items.Add("Channel 15");
			NoteChannelComboBox.Items.Add("Channel 16");
			NoteChannelComboBox.SelectedIndex = 0;
			NoteChannelComboBox.AutoSize = true;
			NoteChannelComboBox.Dock = System.Windows.Forms.DockStyle.Top | System.Windows.Forms.DockStyle.Bottom;
			FlowLayoutPanel.Controls.Add(NoteChannelComboBox);
			FlowLayoutPanel.SetFlowBreak(NoteChannelComboBox, true);

			TransposeRadioButton = new System.Windows.Forms.RadioButton();
			TransposeRadioButton.Text = "Transpose";
			TransposeRadioButton.AutoSize = true;
			TransposeRadioButton.Dock = System.Windows.Forms.DockStyle.Top | System.Windows.Forms.DockStyle.Bottom;
			FlowLayoutPanel.Controls.Add(TransposeRadioButton);
			FlowLayoutPanel.SetFlowBreak(TransposeRadioButton, false);

			TransposeNumericUpDown = new System.Windows.Forms.NumericUpDown();
			TransposeNumericUpDown.Minimum = -128;
			TransposeNumericUpDown.Maximum = 128;
			TransposeNumericUpDown.Value = 0;
			TransposeNumericUpDown.AutoSize = true;
			TransposeNumericUpDown.Dock = System.Windows.Forms.DockStyle.Top | System.Windows.Forms.DockStyle.Bottom;
			FlowLayoutPanel.Controls.Add(TransposeNumericUpDown);
			FlowLayoutPanel.SetFlowBreak(TransposeNumericUpDown, false);

			TransposeMomentaryCheckBox = new System.Windows.Forms.CheckBox();
			TransposeMomentaryCheckBox.Text = "Momentary";
			TransposeMomentaryCheckBox.AutoSize = true;
			TransposeMomentaryCheckBox.Dock = System.Windows.Forms.DockStyle.Top | System.Windows.Forms.DockStyle.Bottom;
			FlowLayoutPanel.Controls.Add(TransposeMomentaryCheckBox);
			FlowLayoutPanel.SetFlowBreak(TransposeMomentaryCheckBox, false);

			TransposeAbsoluteCheckBox = new System.Windows.Forms.CheckBox();
			TransposeAbsoluteCheckBox.Text = "Absolute";
			TransposeAbsoluteCheckBox.AutoSize = true;
			TransposeAbsoluteCheckBox.Dock = System.Windows.Forms.DockStyle.Top | System.Windows.Forms.DockStyle.Bottom;
			FlowLayoutPanel.Controls.Add(TransposeAbsoluteCheckBox);
			FlowLayoutPanel.SetFlowBreak(TransposeAbsoluteCheckBox, true);

			VelocityRadioButton = new System.Windows.Forms.RadioButton();
			VelocityRadioButton.Text = "Velocity";
			VelocityRadioButton.AutoSize = true;
			VelocityRadioButton.Dock = System.Windows.Forms.DockStyle.Top | System.Windows.Forms.DockStyle.Bottom;
			FlowLayoutPanel.Controls.Add(VelocityRadioButton);
			FlowLayoutPanel.SetFlowBreak(VelocityRadioButton, false);

			VelocityNumericUpDown = new System.Windows.Forms.NumericUpDown();
			VelocityNumericUpDown.Minimum = -128;
			VelocityNumericUpDown.Maximum = 128;
			VelocityNumericUpDown.Value = 0;
			VelocityNumericUpDown.AutoSize = true;
			VelocityNumericUpDown.Dock = System.Windows.Forms.DockStyle.Top | System.Windows.Forms.DockStyle.Bottom;
			FlowLayoutPanel.Controls.Add(VelocityNumericUpDown);
			FlowLayoutPanel.SetFlowBreak(VelocityNumericUpDown, false);

			VelocityMomentaryCheckBox = new System.Windows.Forms.CheckBox();
			VelocityMomentaryCheckBox.Text = "Momentary";
			VelocityMomentaryCheckBox.AutoSize = true;
			VelocityMomentaryCheckBox.Dock = System.Windows.Forms.DockStyle.Top | System.Windows.Forms.DockStyle.Bottom;
			FlowLayoutPanel.Controls.Add(VelocityMomentaryCheckBox);
			FlowLayoutPanel.SetFlowBreak(VelocityMomentaryCheckBox, false);

			VelocityAbsoluteCheckBox = new System.Windows.Forms.CheckBox();
			VelocityAbsoluteCheckBox.Text = "Absolute";
			VelocityAbsoluteCheckBox.AutoSize = true;
			VelocityAbsoluteCheckBox.Dock = System.Windows.Forms.DockStyle.Top | System.Windows.Forms.DockStyle.Bottom;
			FlowLayoutPanel.Controls.Add(VelocityAbsoluteCheckBox);
			FlowLayoutPanel.SetFlowBreak(VelocityAbsoluteCheckBox, true);

			PanicRadioButton = new System.Windows.Forms.RadioButton();
			PanicRadioButton.Text = "Panic";
			PanicRadioButton.AutoSize = true;
			PanicRadioButton.Dock = System.Windows.Forms.DockStyle.Top | System.Windows.Forms.DockStyle.Bottom;
			FlowLayoutPanel.Controls.Add(PanicRadioButton);
			FlowLayoutPanel.SetFlowBreak(PanicRadioButton, true);

			InfoRichTextBox = new System.Windows.Forms.RichTextBox();
			InfoRichTextBox.Dock = System.Windows.Forms.DockStyle.Fill;
			InfoRichTextBox.Visible = false;
			SplitContainer.Panel2.Controls.Add(InfoRichTextBox);
		}
	}
}

