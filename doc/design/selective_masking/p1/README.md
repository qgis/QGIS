# Selective masking case 1

<table border=1>
      <tr>
        <td>Layers</td>
        <td><b>Polys</b></td>
        <td><b>Lines</b></td>
        <td><b>Points</b></td>
        <td><b>Labels</b></td>
      </tr>
      <tr>
        <td>Symbol layers</td>
        <td></td>
        <td><b>0</b>: black roads<br/><b>1</b>: yellow roads</td>
        <td><b>0</b>: black jets<br/><b>1</b>: orange jets<br/><b>2</b>: other black jets</td>
        <td></td>
      </tr>
      <tr>
        <td>Masks</td>
        <td></td>
        <td></td>
        <td></td>
        <td>A buffer that masks<br/><b>Lines/0</b><br/><b>Points/0</b><br/><b>Points/2</b></td>
      </tr>
      <tr>
        <td>First pass</td>
        <td><img src="first_pass_0.png"/></td>
        <td><img src="first_pass_1.png"/></td>
        <td><img src="first_pass_2.png"/></td>
        <td><img src="labels.png"/></td>
      </tr>
      <tr>
        <td>Mask image<br/>(still in first pass)</td>
        <td></td>
        <td></td>
        <td></td>
        <td><img src="labels_mask.png"/></td>
      </tr>
    </table>
    <table border=1>
      <tr>
        <td>Second pass</td>
        <td><b>Lines</b> without <b>0</b><br/><img src="second_1.png"/></td>
        <td><b>Points</b> without <b>0, 2</b><br/><img src="second_2.png"/></td>
      </tr>
      <tr>
        <td>Composition with mask</td>
        <td><img src="labels_mask.png"/></td>
        <td><img src="labels_mask.png"/></td>
      </tr>
      <tr>
        <td>Second pass image <b>inside</b> mask</td>
        <td><img src="second_1_a.png"/></td>
        <td><img src="second_2_a.png"/></td>
      </tr>
      <tr>
        <td>First pass image <b>oustide</b> mask</td>
        <td><img src="second_1_first_pass_2.png"/></td>
        <td><img src="second_2_first_pass_2.png"/></td>
      </tr>
      <tr>
        <td>Updated first pass image</td>
        <td><img src="second_1_first_pass_3.png"/></td>
        <td><img src="second_2_first_pass_3.png"/></td>
      </tr>
</table>

Final image <img src="final.png"/>
