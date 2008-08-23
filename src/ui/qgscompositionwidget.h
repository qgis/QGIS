<ui version="4.0" >
 <class>QgsCompositionBase</class>
 <widget class="QWidget" name="QgsCompositionBase" >
  <property name="geometry" >
   <rect>
    <x>0</x>
    <y>0</y>
    <width>243</width>
    <height>422</height>
   </rect>
  </property>
  <property name="sizePolicy" >
   <sizepolicy>
    <hsizetype>7</hsizetype>
    <vsizetype>7</vsizetype>
    <horstretch>0</horstretch>
    <verstretch>0</verstretch>
   </sizepolicy>
  </property>
  <property name="windowTitle" >
   <string>Composition</string>
  </property>
  <layout class="QGridLayout" >
   <property name="margin" >
    <number>9</number>
   </property>
   <property name="spacing" >
    <number>6</number>
   </property>
   <item row="0" column="0" colspan="2" >
    <widget class="QGroupBox" name="groupBox" >
     <property name="title" >
      <string>Paper</string>
     </property>
     <layout class="QGridLayout" >
      <property name="margin" >
       <number>9</number>
      </property>
      <property name="spacing" >
       <number>6</number>
      </property>
      <item row="4" column="1" >
       <widget class="QComboBox" name="mPaperOrientationComboBox" >
        <property name="enabled" >
         <bool>true</bool>
        </property>
        <property name="sizePolicy" >
         <sizepolicy>
          <hsizetype>7</hsizetype>
          <vsizetype>0</vsizetype>
          <horstretch>0</horstretch>
          <verstretch>0</verstretch>
         </sizepolicy>
        </property>
       </widget>
      </item>
      <item row="3" column="1" >
       <widget class="QLineEdit" name="mPaperHeightLineEdit" />
      </item>
      <item row="2" column="1" >
       <widget class="QLineEdit" name="mPaperWidthLineEdit" />
      </item>
      <item row="1" column="1" >
       <widget class="QComboBox" name="mPaperUnitsComboBox" >
        <property name="enabled" >
         <bool>false</bool>
        </property>
        <property name="sizePolicy" >
         <sizepolicy>
          <hsizetype>7</hsizetype>
          <vsizetype>0</vsizetype>
          <horstretch>0</horstretch>
          <verstretch>0</verstretch>
         </sizepolicy>
        </property>
       </widget>
      </item>
      <item row="0" column="1" >
       <widget class="QComboBox" name="mPaperSizeComboBox" >
        <property name="enabled" >
         <bool>true</bool>
        </property>
        <property name="sizePolicy" >
         <sizepolicy>
          <hsizetype>7</hsizetype>
          <vsizetype>0</vsizetype>
          <horstretch>0</horstretch>
          <verstretch>0</verstretch>
         </sizepolicy>
        </property>
       </widget>
      </item>
      <item row="4" column="0" >
       <widget class="QLabel" name="textLabel7" >
        <property name="sizePolicy" >
         <sizepolicy>
          <hsizetype>5</hsizetype>
          <vsizetype>5</vsizetype>
          <horstretch>0</horstretch>
          <verstretch>0</verstretch>
         </sizepolicy>
        </property>
        <property name="text" >
         <string>Orientation</string>
        </property>
        <property name="buddy" >
         <cstring>mPaperOrientationComboBox</cstring>
        </property>
       </widget>
      </item>
      <item row="3" column="0" >
       <widget class="QLabel" name="textLabel6" >
        <property name="sizePolicy" >
         <sizepolicy>
          <hsizetype>5</hsizetype>
          <vsizetype>5</vsizetype>
          <horstretch>0</horstretch>
          <verstretch>0</verstretch>
         </sizepolicy>
        </property>
        <property name="text" >
         <string>Height</string>
        </property>
        <property name="buddy" >
         <cstring>mPaperHeightLineEdit</cstring>
        </property>
       </widget>
      </item>
      <item row="2" column="0" >
       <widget class="QLabel" name="textLabel4" >
        <property name="sizePolicy" >
         <sizepolicy>
          <hsizetype>5</hsizetype>
          <vsizetype>5</vsizetype>
          <horstretch>0</horstretch>
          <verstretch>0</verstretch>
         </sizepolicy>
        </property>
        <property name="text" >
         <string>Width</string>
        </property>
        <property name="buddy" >
         <cstring>mPaperWidthLineEdit</cstring>
        </property>
       </widget>
      </item>
      <item row="1" column="0" >
       <widget class="QLabel" name="textLabel5" >
        <property name="sizePolicy" >
         <sizepolicy>
          <hsizetype>5</hsizetype>
          <vsizetype>5</vsizetype>
          <horstretch>0</horstretch>
          <verstretch>0</verstretch>
         </sizepolicy>
        </property>
        <property name="text" >
         <string>Units</string>
        </property>
        <property name="buddy" >
         <cstring>mPaperUnitsComboBox</cstring>
        </property>
       </widget>
      </item>
      <item row="0" column="0" >
       <widget class="QLabel" name="textLabel3" >
        <property name="sizePolicy" >
         <sizepolicy>
          <hsizetype>5</hsizetype>
          <vsizetype>5</vsizetype>
          <horstretch>0</horstretch>
          <verstretch>0</verstretch>
         </sizepolicy>
        </property>
        <property name="text" >
         <string>Size</string>
        </property>
        <property name="buddy" >
         <cstring>mPaperSizeComboBox</cstring>
        </property>
       </widget>
      </item>
     </layout>
    </widget>
   </item>
   <item row="2" column="0" colspan="2" >
    <spacer>
     <property name="orientation" >
      <enum>Qt::Vertical</enum>
     </property>
     <property name="sizeType" >
      <enum>QSizePolicy::Expanding</enum>
     </property>
     <property name="sizeHint" >
      <size>
       <width>223</width>
       <height>41</height>
      </size>
     </property>
    </spacer>
   </item>
   <item row="1" column="0" >
    <widget class="QLabel" name="textLabel1" >
     <property name="sizePolicy" >
      <sizepolicy>
       <hsizetype>5</hsizetype>
       <vsizetype>5</vsizetype>
       <horstretch>0</horstretch>
       <verstretch>0</verstretch>
      </sizepolicy>
     </property>
     <property name="text" >
      <string>Resolution (dpi)</string>
     </property>
     <property name="buddy" >
      <cstring>mResolutionLineEdit</cstring>
     </property>
    </widget>
   </item>
   <item row="1" column="1" >
    <widget class="QLineEdit" name="mResolutionLineEdit" />
   </item>
  </layout>
 </widget>
 <layoutdefault spacing="6" margin="11" />
 <resources/>
 <connections/>
</ui>
