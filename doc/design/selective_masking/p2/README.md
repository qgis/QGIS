# Selective masking case 2

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
        <td><b>0</b>: orange circles<br/><b>1</b>: marker mask, bigger circle<br/>(only visible in mask image)</td>
        <td></td>
      </tr>
      <tr>
        <td>Masks</td>
        <td></td>
        <td></td>
        <td><b>Points/1</b> masks <b>Lines/0</b></td>
        <td></td>
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
        <td><img src="first_pass_2_mask.png"/></td>
        <td></td>
      </tr>
    </table>
    <table border=1>
      <tr>
        <td>Second pass</td>
        <td><b>Lines</b> without <b>0</b><br/><img src="second_1.png"/></td>
      </tr>
      <tr>
        <td>Composition with mask</td>
        <td><img src="first_pass_2_mask.png"/></td>
      </tr>
      <tr>
        <td>Second pass image <b>inside</b> mask</td>
        <td><img src="second_1_a.png"/></td>
      </tr>
      <tr>
        <td>First pass image <b>oustide</b> mask</td>
        <td><img src="second_1_first_pass_2.png"/></td>
      </tr>
      <tr>
        <td>Updated first pass image</td>
        <td><img src="second_1_first_pass_3.png"/></td>
      </tr>
</table>

Final image <img src="final.png"/>

