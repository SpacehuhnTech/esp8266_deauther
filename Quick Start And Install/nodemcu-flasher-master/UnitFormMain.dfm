object FormMain: TFormMain
  Left = 221
  Top = 103
  AlphaBlend = True
  Caption = 'NODEMCU FIRMWARE PROGRAMMER'
  ClientHeight = 298
  ClientWidth = 556
  Color = clCream
  TransparentColorValue = clNone
  Constraints.MinWidth = 525
  DoubleBuffered = True
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -21
  Font.Name = #24494#36719#38597#40657
  Font.Style = []
  GlassFrame.Top = 30
  OldCreateOrder = False
  OnClose = FormClose
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  PixelsPerInch = 96
  TextHeight = 28
  object PanelControlPanel: TPanel
    Left = 0
    Top = 271
    Width = 556
    Height = 27
    Align = alBottom
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -21
    Font.Name = #24494#36719#38597#40657
    Font.Style = []
    ParentColor = True
    ParentFont = False
    TabOrder = 0
    object LabelCopyRight: TLabel
      Left = 28
      Top = 2
      Width = 296
      Height = 20
      Caption = 'NodeMCU Team <vowstar@nodemcu.com>'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -14
      Font.Name = #24494#36719#38597#40657
      Font.Style = []
      ParentFont = False
    end
    object ImageStatus: TImage
      Left = 1
      Top = 1
      Width = 25
      Height = 25
      Align = alLeft
      Stretch = True
      Transparent = True
      ExplicitLeft = 484
      ExplicitTop = 2
    end
    object LabelStatus: TLabel
      Left = 514
      Top = 1
      Width = 41
      Height = 25
      Align = alRight
      Caption = 'Ready'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -14
      Font.Name = #24494#36719#38597#40657
      Font.Style = []
      ParentFont = False
      ExplicitHeight = 20
    end
  end
  object PageControlMain: TPageControl
    Left = 0
    Top = 0
    Width = 556
    Height = 271
    ActivePage = TabSheetOperation
    Align = alClient
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -21
    Font.Name = #24494#36719#38597#40657
    Font.Style = []
    ParentFont = False
    TabOrder = 1
    object TabSheetOperation: TTabSheet
      Caption = 'Operation'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -21
      Font.Name = #24494#36719#38597#40657
      Font.Style = []
      ParentFont = False
      DesignSize = (
        548
        228)
      object LabelDevA: TLabel
        Left = 13
        Top = 20
        Width = 100
        Height = 28
        Caption = 'COM Port'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -21
        Font.Name = #24494#36719#38597#40657
        Font.Style = []
        ParentFont = False
      end
      object LabelIntroduction: TLabel
        Left = 13
        Top = 97
        Width = 146
        Height = 20
        Cursor = crHandPoint
        Caption = 'require("nodemcu")'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = #24494#36719#38597#40657
        Font.Style = []
        ParentFont = False
        StyleElements = []
        OnClick = LabelIntroductionClick
      end
      object ComboBoxSerialPortA: TComboBox
        Left = 119
        Top = 16
        Width = 162
        Height = 36
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -21
        Font.Name = #24494#36719#38597#40657
        Font.Style = []
        ImeName = #20013#25991'('#31616#20307') - '#25628#29399#25340#38899#36755#20837#27861
        ParentColor = True
        ParentFont = False
        ParentShowHint = False
        ShowHint = True
        TabOrder = 0
        TabStop = False
        TextHint = 'Select port'
      end
      object ButtonBurn: TButton
        Left = 297
        Top = 16
        Width = 233
        Height = 36
        Action = ActionBurn
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Flash(&F)'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -21
        Font.Name = #24494#36719#38597#40657
        Font.Style = []
        ParentFont = False
        TabOrder = 1
      end
      object ProgressBarStatus: TProgressBar
        Left = 13
        Top = 58
        Width = 517
        Height = 33
        Anchors = [akLeft, akTop, akRight]
        Smooth = True
        TabOrder = 2
      end
      object PanelQRCode: TPanel
        Left = 13
        Top = 123
        Width = 100
        Height = 100
        TabOrder = 3
        object ImageQRCode: TImage
          Left = 1
          Top = 1
          Width = 98
          Height = 98
          Align = alClient
          Stretch = True
          ExplicitLeft = 0
          ExplicitTop = -23
        end
      end
      object LabeledEditAPMAC: TLabeledEdit
        Left = 216
        Top = 124
        Width = 314
        Height = 36
        Anchors = [akLeft, akTop, akRight]
        EditLabel.Width = 84
        EditLabel.Height = 28
        EditLabel.Caption = 'AP MAC'
        LabelPosition = lpLeft
        ReadOnly = True
        TabOrder = 4
        Text = 'Waiting MAC'
        TextHint = 'Waiting MAC'
      end
      object LabeledEditSTAMAC: TLabeledEdit
        Left = 216
        Top = 184
        Width = 314
        Height = 36
        Anchors = [akLeft, akTop, akRight]
        EditLabel.Width = 95
        EditLabel.Height = 28
        EditLabel.Caption = 'STA MAC'
        LabelPosition = lpLeft
        ReadOnly = True
        TabOrder = 5
        Text = 'Waiting MAC'
        TextHint = 'Waiting MAC'
      end
    end
    object TabSheetConfig: TTabSheet
      Caption = 'Config'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -21
      Font.Name = #24494#36719#38597#40657
      Font.Style = []
      ImageIndex = 3
      ParentFont = False
      object GridPanelConfig: TGridPanel
        Left = 0
        Top = 0
        Width = 548
        Height = 228
        Align = alClient
        ColumnCollection = <
          item
            Value = 100.000000000000000000
          end>
        ControlCollection = <
          item
            Column = 0
            Control = FrameConfigLine1
            Row = 0
          end
          item
            Column = 0
            Control = FrameConfigLine2
            Row = 1
          end
          item
            Column = 0
            Control = FrameConfigLine3
            Row = 2
          end
          item
            Column = 0
            Control = FrameConfigLine4
            Row = 3
          end
          item
            Column = 0
            Control = FrameConfigLine5
            Row = 4
          end
          item
            Column = 0
            Control = FrameConfigLine6
            Row = 5
          end
          item
            Column = 0
            Control = FrameConfigLine7
            Row = 6
          end>
        RowCollection = <
          item
            SizeStyle = ssAbsolute
            Value = 30.000000000000000000
          end
          item
            SizeStyle = ssAbsolute
            Value = 30.000000000000000000
          end
          item
            SizeStyle = ssAbsolute
            Value = 30.000000000000000000
          end
          item
            SizeStyle = ssAbsolute
            Value = 30.000000000000000000
          end
          item
            SizeStyle = ssAbsolute
            Value = 30.000000000000000000
          end
          item
            SizeStyle = ssAbsolute
            Value = 30.000000000000000000
          end
          item
            SizeStyle = ssAbsolute
            Value = 30.000000000000000000
          end
          item
            SizeStyle = ssAbsolute
            Value = 30.000000000000000000
          end>
        TabOrder = 0
        inline FrameConfigLine1: TFrameConfigLine
          Left = 1
          Top = 1
          Width = 546
          Height = 31
          Align = alTop
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'Tahoma'
          Font.Style = []
          ParentFont = False
          TabOrder = 0
          ExplicitLeft = 1
          ExplicitTop = 1
          ExplicitWidth = 546
          inherited GridPanelConfigLine: TGridPanel
            Width = 546
            ControlCollection = <
              item
                Column = 0
                Control = FrameConfigLine1.CheckBoxEnable
                Row = 0
              end
              item
                Column = 1
                Control = FrameConfigLine1.ComboBoxPath
                Row = 0
              end
              item
                Column = 3
                Control = FrameConfigLine1.ComboBoxOffset
                Row = 0
              end
              item
                Column = 2
                Control = FrameConfigLine1.ImageOpenFile
                Row = 0
              end>
            ExplicitWidth = 546
            inherited CheckBoxEnable: TCheckBox
              OnMouseLeave = FrameConfigLineChange
            end
            inherited ComboBoxPath: TComboBox
              Width = 394
              OnChange = FrameConfigLineChange
              ExplicitWidth = 394
            end
            inherited ComboBoxOffset: TComboBox
              Left = 445
              OnChange = FrameConfigLineChange
              ExplicitLeft = 445
            end
            inherited ImageOpenFile: TImage
              Left = 415
              ExplicitLeft = 368
            end
          end
        end
        inline FrameConfigLine2: TFrameConfigLine
          Left = 1
          Top = 31
          Width = 546
          Height = 31
          Align = alTop
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'Tahoma'
          Font.Style = []
          ParentFont = False
          TabOrder = 1
          ExplicitLeft = 1
          ExplicitTop = 31
          ExplicitWidth = 546
          inherited GridPanelConfigLine: TGridPanel
            Width = 546
            ControlCollection = <
              item
                Column = 0
                Control = FrameConfigLine2.CheckBoxEnable
                Row = 0
              end
              item
                Column = 1
                Control = FrameConfigLine2.ComboBoxPath
                Row = 0
              end
              item
                Column = 3
                Control = FrameConfigLine2.ComboBoxOffset
                Row = 0
              end
              item
                Column = 2
                Control = FrameConfigLine2.ImageOpenFile
                Row = 0
              end>
            ExplicitWidth = 546
            inherited CheckBoxEnable: TCheckBox
              OnMouseLeave = FrameConfigLineChange
            end
            inherited ComboBoxPath: TComboBox
              Width = 394
              OnChange = FrameConfigLineChange
              ExplicitWidth = 394
            end
            inherited ComboBoxOffset: TComboBox
              Left = 445
              OnChange = FrameConfigLineChange
              ExplicitLeft = 445
            end
            inherited ImageOpenFile: TImage
              Left = 415
              ExplicitLeft = 368
            end
          end
        end
        inline FrameConfigLine3: TFrameConfigLine
          Left = 1
          Top = 61
          Width = 546
          Height = 30
          Align = alTop
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'Tahoma'
          Font.Style = []
          ParentFont = False
          TabOrder = 2
          ExplicitLeft = 1
          ExplicitTop = 61
          ExplicitWidth = 546
          ExplicitHeight = 30
          inherited GridPanelConfigLine: TGridPanel
            Width = 546
            Height = 30
            ControlCollection = <
              item
                Column = 0
                Control = FrameConfigLine3.CheckBoxEnable
                Row = 0
              end
              item
                Column = 1
                Control = FrameConfigLine3.ComboBoxPath
                Row = 0
              end
              item
                Column = 3
                Control = FrameConfigLine3.ComboBoxOffset
                Row = 0
              end
              item
                Column = 2
                Control = FrameConfigLine3.ImageOpenFile
                Row = 0
              end>
            ExplicitWidth = 546
            ExplicitHeight = 30
            inherited CheckBoxEnable: TCheckBox
              Height = 28
              OnMouseLeave = FrameConfigLineChange
              ExplicitHeight = 28
            end
            inherited ComboBoxPath: TComboBox
              Width = 394
              OnChange = FrameConfigLineChange
              ExplicitWidth = 394
            end
            inherited ComboBoxOffset: TComboBox
              Left = 445
              OnChange = FrameConfigLineChange
              ExplicitLeft = 445
            end
            inherited ImageOpenFile: TImage
              Left = 415
              Height = 28
              ExplicitLeft = 368
              ExplicitHeight = 28
            end
          end
        end
        inline FrameConfigLine4: TFrameConfigLine
          Left = 1
          Top = 91
          Width = 546
          Height = 30
          Align = alTop
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'Tahoma'
          Font.Style = []
          ParentFont = False
          TabOrder = 3
          ExplicitLeft = 1
          ExplicitTop = 91
          ExplicitWidth = 546
          ExplicitHeight = 30
          inherited GridPanelConfigLine: TGridPanel
            Width = 546
            Height = 30
            ControlCollection = <
              item
                Column = 0
                Control = FrameConfigLine4.CheckBoxEnable
                Row = 0
              end
              item
                Column = 1
                Control = FrameConfigLine4.ComboBoxPath
                Row = 0
              end
              item
                Column = 3
                Control = FrameConfigLine4.ComboBoxOffset
                Row = 0
              end
              item
                Column = 2
                Control = FrameConfigLine4.ImageOpenFile
                Row = 0
              end>
            ExplicitWidth = 546
            ExplicitHeight = 30
            inherited CheckBoxEnable: TCheckBox
              Height = 28
              OnMouseLeave = FrameConfigLineChange
              ExplicitHeight = 28
            end
            inherited ComboBoxPath: TComboBox
              Width = 394
              OnChange = FrameConfigLineChange
              ExplicitWidth = 394
            end
            inherited ComboBoxOffset: TComboBox
              Left = 445
              OnChange = FrameConfigLineChange
              ExplicitLeft = 445
            end
            inherited ImageOpenFile: TImage
              Left = 415
              Height = 28
              ExplicitLeft = 368
              ExplicitHeight = 28
            end
          end
        end
        inline FrameConfigLine5: TFrameConfigLine
          Left = 1
          Top = 121
          Width = 546
          Height = 30
          Align = alTop
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'Tahoma'
          Font.Style = []
          ParentFont = False
          TabOrder = 4
          ExplicitLeft = 1
          ExplicitTop = 121
          ExplicitWidth = 546
          ExplicitHeight = 30
          inherited GridPanelConfigLine: TGridPanel
            Width = 546
            Height = 30
            ControlCollection = <
              item
                Column = 0
                Control = FrameConfigLine5.CheckBoxEnable
                Row = 0
              end
              item
                Column = 1
                Control = FrameConfigLine5.ComboBoxPath
                Row = 0
              end
              item
                Column = 3
                Control = FrameConfigLine5.ComboBoxOffset
                Row = 0
              end
              item
                Column = 2
                Control = FrameConfigLine5.ImageOpenFile
                Row = 0
              end>
            ExplicitWidth = 546
            ExplicitHeight = 30
            inherited CheckBoxEnable: TCheckBox
              Height = 28
              OnMouseLeave = FrameConfigLineChange
              ExplicitHeight = 28
            end
            inherited ComboBoxPath: TComboBox
              Width = 394
              OnChange = FrameConfigLineChange
              ExplicitWidth = 394
            end
            inherited ComboBoxOffset: TComboBox
              Left = 445
              OnChange = FrameConfigLineChange
              ExplicitLeft = 445
            end
            inherited ImageOpenFile: TImage
              Left = 415
              Height = 28
              ExplicitLeft = 368
              ExplicitHeight = 28
            end
          end
        end
        inline FrameConfigLine6: TFrameConfigLine
          Left = 1
          Top = 151
          Width = 546
          Height = 30
          Align = alTop
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'Tahoma'
          Font.Style = []
          ParentFont = False
          TabOrder = 5
          ExplicitLeft = 1
          ExplicitTop = 151
          ExplicitWidth = 546
          ExplicitHeight = 30
          inherited GridPanelConfigLine: TGridPanel
            Width = 546
            Height = 30
            ControlCollection = <
              item
                Column = 0
                Control = FrameConfigLine6.CheckBoxEnable
                Row = 0
              end
              item
                Column = 1
                Control = FrameConfigLine6.ComboBoxPath
                Row = 0
              end
              item
                Column = 3
                Control = FrameConfigLine6.ComboBoxOffset
                Row = 0
              end
              item
                Column = 2
                Control = FrameConfigLine6.ImageOpenFile
                Row = 0
              end>
            ExplicitWidth = 546
            ExplicitHeight = 30
            inherited CheckBoxEnable: TCheckBox
              Height = 28
              OnMouseLeave = FrameConfigLineChange
              ExplicitHeight = 28
            end
            inherited ComboBoxPath: TComboBox
              Width = 394
              OnChange = FrameConfigLineChange
              ExplicitWidth = 394
            end
            inherited ComboBoxOffset: TComboBox
              Left = 445
              OnChange = FrameConfigLineChange
              ExplicitLeft = 445
            end
            inherited ImageOpenFile: TImage
              Left = 415
              Height = 28
              ExplicitLeft = 368
              ExplicitHeight = 28
            end
          end
        end
        inline FrameConfigLine7: TFrameConfigLine
          Left = 1
          Top = 181
          Width = 546
          Height = 30
          Align = alTop
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'Tahoma'
          Font.Style = []
          ParentFont = False
          TabOrder = 6
          ExplicitLeft = 1
          ExplicitTop = 181
          ExplicitWidth = 546
          ExplicitHeight = 30
          inherited GridPanelConfigLine: TGridPanel
            Width = 546
            Height = 30
            ControlCollection = <
              item
                Column = 0
                Control = FrameConfigLine7.CheckBoxEnable
                Row = 0
              end
              item
                Column = 1
                Control = FrameConfigLine7.ComboBoxPath
                Row = 0
              end
              item
                Column = 3
                Control = FrameConfigLine7.ComboBoxOffset
                Row = 0
              end
              item
                Column = 2
                Control = FrameConfigLine7.ImageOpenFile
                Row = 0
              end>
            ExplicitWidth = 546
            ExplicitHeight = 30
            inherited CheckBoxEnable: TCheckBox
              Height = 28
              OnMouseLeave = FrameConfigLineChange
              ExplicitHeight = 28
            end
            inherited ComboBoxPath: TComboBox
              Width = 394
              OnChange = FrameConfigLineChange
              ExplicitWidth = 394
            end
            inherited ComboBoxOffset: TComboBox
              Left = 445
              OnChange = FrameConfigLineChange
              ExplicitLeft = 445
            end
            inherited ImageOpenFile: TImage
              Left = 415
              Height = 28
              ExplicitLeft = 368
              ExplicitHeight = 28
            end
          end
        end
      end
    end
    object TabSheetAdvanced: TTabSheet
      Caption = 'Advanced'
      ImageIndex = 4
      object LabelFlashBaudrate: TLabel
        Left = 16
        Top = 27
        Width = 91
        Height = 28
        Caption = 'Baudrate'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -21
        Font.Name = #24494#36719#38597#40657
        Font.Style = []
        ParentFont = False
      end
      object LabelFlashSize: TLabel
        Left = 16
        Top = 69
        Width = 96
        Height = 28
        Caption = 'Flash size'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -21
        Font.Name = #24494#36719#38597#40657
        Font.Style = []
        ParentFont = False
      end
      object LabelFlashSpeed: TLabel
        Left = 16
        Top = 111
        Width = 118
        Height = 28
        Caption = 'Flash speed'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -21
        Font.Name = #24494#36719#38597#40657
        Font.Style = []
        ParentFont = False
      end
      object LabelSPIMode: TLabel
        Left = 16
        Top = 153
        Width = 96
        Height = 28
        Caption = 'SPI Mode'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -21
        Font.Name = #24494#36719#38597#40657
        Font.Style = []
        ParentFont = False
      end
      object ComboBoxFlashBaudrate: TComboBox
        Left = 140
        Top = 24
        Width = 162
        Height = 36
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -21
        Font.Name = #24494#36719#38597#40657
        Font.Style = []
        ImeName = #20013#25991'('#31616#20307') - '#25628#29399#25340#38899#36755#20837#27861
        ParentColor = True
        ParentFont = False
        ParentShowHint = False
        ShowHint = True
        TabOrder = 0
        TabStop = False
        Text = '230400'
        TextHint = 'Baudrate'
        OnChange = FrameConfigLineChange
        Items.Strings = (
          '9600'
          '19200'
          '38400'
          '57600'
          '74880'
          '115200'
          '230400'
          '460800'
          '512000'
          '576000'
          '921600')
      end
      object ComboBoxFlashSize: TComboBox
        Left = 140
        Top = 66
        Width = 162
        Height = 36
        Style = csDropDownList
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -21
        Font.Name = #24494#36719#38597#40657
        Font.Style = []
        ImeName = #20013#25991'('#31616#20307') - '#25628#29399#25340#38899#36755#20837#27861
        ItemIndex = 4
        ParentColor = True
        ParentFont = False
        ParentShowHint = False
        ShowHint = True
        TabOrder = 1
        TabStop = False
        Text = '4MByte'
        TextHint = 'Select size'
        OnChange = FrameConfigLineChange
        Items.Strings = (
          '256kByte'
          '512kByte'
          '1MByte'
          '2MByte'
          '4MByte'
          '8MByte'
          '16MByte')
      end
      object ComboBoxFlashSpeed: TComboBox
        Left = 140
        Top = 108
        Width = 162
        Height = 36
        Style = csDropDownList
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -21
        Font.Name = #24494#36719#38597#40657
        Font.Style = []
        ImeName = #20013#25991'('#31616#20307') - '#25628#29399#25340#38899#36755#20837#27861
        ItemIndex = 0
        ParentColor = True
        ParentFont = False
        ParentShowHint = False
        ShowHint = True
        TabOrder = 2
        TabStop = False
        Text = '40MHz'
        TextHint = 'Select speed'
        OnChange = FrameConfigLineChange
        Items.Strings = (
          '40MHz'
          '26.7MHz'
          '20MHz'
          '80MHz')
      end
      object ButtonRestoreDefault: TButton
        Left = 328
        Top = 24
        Width = 198
        Height = 162
        Caption = 'Restore default'
        TabOrder = 3
        OnClick = ButtonRestoreDefaultClick
      end
      object ComboBoxSPIMode: TComboBox
        Left = 140
        Top = 150
        Width = 162
        Height = 36
        Style = csDropDownList
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -21
        Font.Name = #24494#36719#38597#40657
        Font.Style = []
        ImeName = #20013#25991'('#31616#20307') - '#25628#29399#25340#38899#36755#20837#27861
        ItemIndex = 1
        ParentColor = True
        ParentFont = False
        ParentShowHint = False
        ShowHint = True
        TabOrder = 4
        TabStop = False
        Text = 'DIO'
        TextHint = 'Select speed'
        OnChange = FrameConfigLineChange
        Items.Strings = (
          'QIO'
          'DIO'
          'QOUT'
          'DOUT')
      end
    end
    object TabSheetIntroduction: TTabSheet
      Caption = 'About'
      ImageIndex = 2
      object RichEditNote: TRichEdit
        Left = 0
        Top = 0
        Width = 548
        Height = 228
        Align = alClient
        BevelInner = bvNone
        BevelOuter = bvNone
        BorderStyle = bsNone
        Ctl3D = False
        Font.Charset = GB2312_CHARSET
        Font.Color = clWindowText
        Font.Height = -14
        Font.Name = #24494#36719#38597#40657
        Font.Style = []
        Lines.Strings = (
          'If you have any questions, please contact me.'
          'Vowstar <vowstar@gmail.com>'
          'NodeMcu is a powerful tool for software engineers'
          'to program from HardwareIO to Wifi network within'
          'several minites.'
          'This programmer can flash esp8266 by one click.'
          'Our website is http://www.nodemcu.com'
          'Our Tencent QQ Group:309957875.'
          '')
        ParentColor = True
        ParentCtl3D = False
        ParentFont = False
        ReadOnly = True
        ScrollBars = ssBoth
        TabOrder = 0
        Zoom = 100
      end
    end
    object TabSheetLog: TTabSheet
      Caption = 'Log'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -21
      Font.Name = #24494#36719#38597#40657
      Font.Style = []
      ImageIndex = 1
      ParentFont = False
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object MemoOutput: TMemo
        Left = 0
        Top = 0
        Width = 548
        Height = 228
        Hint = #36816#34892#36755#20986'.|'#23547#21271#31243#24207#30340#36816#34892#36755#20986#22312#36825#37324#26174#31034'.'
        Align = alClient
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -14
        Font.Name = #24494#36719#38597#40657
        Font.Style = []
        ImeName = #20013#25991'('#31616#20307') - '#25628#29399#25340#38899#36755#20837#27861
        ParentColor = True
        ParentFont = False
        ParentShowHint = False
        ReadOnly = True
        ScrollBars = ssVertical
        ShowHint = True
        TabOrder = 0
        OnChange = MemoOutputChange
      end
    end
  end
  object ActionListStandard: TActionList
    Left = 272
    object FileExit: TFileExit
      Category = 'File'
      Caption = #36864#20986'(&X)'
      Hint = 'Exit|Quits the application'
      ImageIndex = 43
    end
    object ActionBurn: TAction
      Category = 'Control'
      Caption = #19968#38190#28903#20889'(&F)'
      OnExecute = ActionBurnExecute
    end
  end
  object TimerFindPorts: TTimer
    Interval = 500
    OnTimer = TimerFindPortsTimer
    Left = 448
  end
  object TimerStateMachine: TTimer
    Enabled = False
    Interval = 50
    OnTimer = TimerStateMachineTimer
    Left = 368
  end
  object IdHTTPUpdate: TIdHTTP
    AllowCookies = True
    ProxyParams.BasicAuthentication = False
    ProxyParams.ProxyPort = 0
    Request.ContentLength = -1
    Request.ContentRangeEnd = -1
    Request.ContentRangeStart = -1
    Request.ContentRangeInstanceLength = -1
    Request.Accept = #9'text/html, application/xhtml+xml, */*'
    Request.AcceptCharSet = 'utf-8'
    Request.BasicAuthentication = False
    Request.UserAgent = 
      'Mozilla/5.0 (Windows NT 6.3; ARM; Trident/7.0; Touch; rv:11.0) l' +
      'ike Gecko'
    Request.Ranges.Units = 'bytes'
    Request.Ranges = <>
    HTTPOptions = []
    Left = 512
  end
  object TimerCode: TTimer
    Interval = 100
    OnTimer = TimerCodeTimer
    Left = 496
    Top = 112
  end
end
