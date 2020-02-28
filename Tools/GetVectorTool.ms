function GetVector vector_out selectedMode = 
(
	vector_out.text = ""
			
	for i in selection do
	(
		case selectedMode of
		(
			----------------------------------------------------------------------------------------------------------------------------------- C++ style single _V (for orbiter)
			1: vector_out.text += i.name + ">> \n\t" + ("_V("+((-i.pivot.x) as string)+", "+(i.pivot.z as string)+", "+((-i.pivot.y) as string)+")\n\n")
					
			----------------------------------------------------------------------------------------------------------------------------------- DXS javascript style
			2: vector_out.text += i.name + ">> \n\t" + ("new Vector("+((-i.pivot.x)  as string)+", "+(i.pivot.z as string)+", "+((-i.pivot.y) as string)+");\n\n")
			
			----------------------------------------------------------------------------------------------------------------------------------- DXS named vector consts
			3: vector_out.text += ("const V_"+i.name+" = new Vector("+((-i.pivot.x) as string)+", "+(i.pivot.z as string)+", "+((-i.pivot.y) as string)+");\n")
					
			------------------------------------------------------------------------------------------------------------------------ CFG file entry (space separated)
			4: vector_out.text += i.name + ">> \n\t" + ((-i.pivot.x)  as string)+" "+(i.pivot.z as string)+" "+((-i.pivot.y) as string)+"\n\n"
					
			----------------------------------------------------------------------------------------------------------------------------------- CFG (with commas)
			5: vector_out.text += i.name + ">> \n\t" + ((-i.pivot.x) as string)+", "+(i.pivot.z as string)+", "+((-i.pivot.y) as string)+"\n\n"
					
			----------------------------------------------------------------------------------------------------------------------------------- V3_ named defines
			6: vector_out.text += "#define V3_"+i.name + " _V(" +((-i.pivot.x) as string)+", "+(i.pivot.z as string)+", "+((-i.pivot.y) as string)+")\n"
					
			----------------------------------------------------------------------------------------------------------------------------------- V3_TDIR targets delta-dir defines
			7: 
			(
				if (i.target != undefined) do 
				(
					axis = normalize (i.target.pivot - i.pivot)
					vector_out.text += "#define V3_TDIR_"+i.name +" _V(" +((-axis.x)  as string)+", "+(axis.z as string)+", "+((-axis.y) as string)+")\n"
				)
			)
			
			----------------------------------------------------------------------------------------------------------------------------------- DXS  targets delta-dir consts
			8: 
			(
				if (i.target != undefined) do (
					axis = normalize (i.target.pivot - i.pivot)
					vector_out.text += "const VDIR_"+i.name +" = new Vector(" +((-axis.x as string)+", "+(axis.z as string)+", "+((-axis.y) as string)+");\n")
				)
			)
			
			----------------------------------------------------------------------------------------------------------------------------------- V3BDBX_ boundnig box defines
			9: 
			(
				bb = nodeLocalBoundingBox i
				vector_out.text += "#define V3_BDBX_A_"+i.name + " _V(" +((-bb[1].x) as string)+", "+(bb[1].z as string)+", "+((-bb[1].y) as string)+")\n"+
			                               "#define V3_BDBX_B_"+i.name + " _V(" +((-bb[2].x) as string)+", "+(bb[2].z as string)+", "+((-bb[2].y) as string)+")\n\n"
			)	
			
			10:(
				if (i.target != undefined) do 
				(
					axis = normalize (i.target.pivot - i.pivot)
					vector_out.text += "#define VHINGE_"+i.name + " _V(" +((-i.pivot.x) as string)+", "+(i.pivot.z as string)+", "+((-i.pivot.y) as string)+"),"+
					" _V(" +((-axis.x)  as string)+", "+(axis.z as string)+", "+((-axis.y) as string)+")\n"
				)						
			)
					
			11: 
			(
					vector_out.text +="_V(" +((-i.pivot.x) as string)+", "+(i.pivot.z as string)+", "+((-i.pivot.y) as string)+"), /* " +  i.name + " */\n"
			)
					
			12: 
			(
				axis = normalize(i.dir)
				vector_out.text += "#define V3_LZD_"+i.name + " _V(" +((-axis.x) as string)+", "+(axis.z as string)+", "+((-axis.y) as string)+")\n"
			)
			
			13: 
			(
				axis = normalize(i.dir)
				vector_out.text += "#define POSDIR_"+i.name + " _V(" +((-i.pivot.x) as string)+", "+(i.pivot.z as string)+", "+((-i.pivot.y) as string)+"), " + 
				"_V(" +((-axis.x) as string)+", "+(axis.z as string)+", "+((-axis.y) as string)+")\n"
			)
		)
	)
)

utility GetVectorTool "Get Vector" 
( 
	button get "Get Vector"
	
	global selectedMode = 1
	
	dropdownlist mode_dd "Format" items:#("C++  _V singles",  "DXS Vector singles", "DXS V_ named consts", \
			"CFG (spaced) single entries", "CFG (commas) single entries", "C++ _V  V3_ named defines *beware of spaces*",\
			"C++ _V V3_TDIR_ defines :: targets delta-vector (normalized)", "DXS VDIR_ consts :: targets delta-vector (normalized)",\
			"C++ _Vx2 V3B_DBX_ A/B defines :: bounding boxes", "C++ VHINGE_ point-and-axis defines", "C++ _V array", "_V3_LZD_ local Z direction #defines ", "POSDIR_ position and direction pair #defines")
		
	on mode_dd selected i do	
		selectedMode = i
	
	editText vector_out "" height:200 enabled:true bold:true readOnly:true
	
	on get pressed do 
		GetVector vector_out selectedMode
)