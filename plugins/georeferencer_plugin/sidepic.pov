global_settings {

}


sky_sphere {
	/*
	pigment { 
		gradient x
		colour_map {
			[0.0 colour <0.7, 0.7, 1.0>]
			[1.0 colour <0.4, 0.4, 1.0>]
		}
		rotate 30*y
	}
	*/
	pigment {
		bozo
		turbulence 3
		colour_map {
			[0.0 colour <0.7, 0.7, 1.0>]
			[1.0 colour <1.0, 1.0, 1.0>]
		}
	}
}


camera {
	location <0, 1,-10>
        look_at  <0, 0, 10>
}


light_source {
	<0, 5,-20>
        colour   <1, 1, 1>
}


plane {
	<0, 1, 0>, 0
	texture {
		pigment {
			gradient x
			color_map {
				[0.0  color  <0,0,1>]
				[0.02  color  <0,0,1>]
				[0.03 color  <1,1,1>]
				[0.97 color  <1,1,1>]
				[0.98  color  <0,0,1>]
				[1.0  color  <0,0,1>]
			}
		}
		finish {
			ambient 0.5
			reflection 0.3
		}
		scale 1
	}
	texture {
		pigment {
			gradient z
			color_map {
				[0.0  color  <0,0,1>]
				[0.02  color  <0,0,1>]
				[0.03 color  <0,0,1,1>]
				[0.97 color  <0,0,1,1>]
				[0.98  color  <0,0,1>]
				[1.0  color  <0,0,1>]
			}
		}
		finish {
			ambient 0.5
			reflection 0.3
		}
		scale 2
	}
	
	rotate <0, 35, 0>
}


box {
	<0,0,0>, <1,1,1>
	texture {
		pigment {
			colour <0.3,0.4,1.0>
		}
		finish {
			ambient 0.5
			reflection 0.2
		}
	}
	texture {
		pigment {
			image_map { tga "sidepic_bg.tga" }
		}
		finish {
			ambient 0.5
		}
	}
	rotate 90*x
	scale <5,1,6>
	translate <-3, 0, -2>
}


#declare pin = union {
	cylinder {
		<0, 0, 0>, <0, 1, 0>, 0.02
		texture {
			pigment {
				colour <0.8, 0.8, 0.8>
			}
			finish {
				ambient 0.2
				phong 0.7
			}			
		}
	}
	sphere {
		<0, 1, 0>, 0.05
		texture {
			pigment {
				colour <1.0, 0, 0>
			}
			finish {
				ambient 0.4
				phong 0.7
			}
		}
	}
}

#declare s = seed(1979374987);

object {
	pin
	translate -(0.1 + 0.3*rand(s))*y
	rotate 30*rand(s)*z
	rotate 360*rand(s)*y
}

object {
	pin
	translate -(0.1 + 0.5*rand(s))*y
	rotate 30*rand(s)*z
	rotate 360*rand(s)*y
	translate <-1, 0, 1>
}	

object {
	pin
	translate -(0.1 + 0.5*rand(s))*y
	rotate 30*rand(s)*z
	rotate 360*rand(s)*y
	translate <-2.6, 0, -1.7>
}	

object {
	pin
	translate -(0.1 + 0.5*rand(s))*y
	rotate 30*rand(s)*z
	rotate 360*rand(s)*y
	translate <1.6, 0, 3.3>
}	

object {
	pin
	translate -(0.1 + 0.5*rand(s))*y
	rotate 30*rand(s)*z
	rotate 360*rand(s)*y
	translate <1.0, 0, -1.3>
}	