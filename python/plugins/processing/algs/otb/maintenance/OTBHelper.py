# -*- coding: utf-8 -*-

"""
***************************************************************************
    OTBHelper.py
    ---------------------
    Copyright            : (C) 2013 by CS Systemes d'information (CS SI)
    Email                : otb at c-s dot fr (CS SI)
    Contributors         : Julien Malik (CS SI)  - File creation
                           Oscar Picas (CS SI)   -
                           Alexia Mondot (CS SI) - Add particular case in xml creation
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
__author__ = 'Julien Malik, Oscar Picas, Alexia Mondot'
__copyright__ = '(C) 2013, CS Systemes d\'information  (CS SI)'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'
__version__ = "3.8"

import os
import copy

import xml.etree.ElementTree as ET
import traceback

from contextlib import contextmanager
import shutil


@contextmanager
def tag(name, c):
    c.append("<%s>" % name)
    yield
    if ' ' in name:
        c.append("</%s>" % name.split(' ')[0])
    else:
        c.append("</%s>" % name)


@contextmanager
def opentag(name, c):
    c.append("<%s>" % name)
    yield


def get_group(appInstance):
    tags = appInstance.GetDocTags()
    sectionTags = ["Image Manipulation", "Vector Data Manipulation", "Calibration", "Geometry", "Image Filtering", "Feature Extraction", "Stereo", "Learning", "Segmentation"]
    for sectionTag in sectionTags:
        for tag in tags:
            if tag == sectionTag:
                return sectionTag
    return "Miscellaneous"


def set_OTB_log():
    import logging
    logger = logging.getLogger('OTBGenerator')
    hdlr = logging.FileHandler('OTBGenerator.log')
    hdlr.setLevel(logging.DEBUG)
    cons = logging.StreamHandler()
    cons.setLevel(logging.CRITICAL)
    formatter = logging.Formatter('%(asctime)s %(levelname)s %(message)s')
    hdlr.setFormatter(formatter)
    logger.addHandler(hdlr)
    logger.addHandler(cons)
    logger.setLevel(logging.DEBUG)


def get_OTB_log():
    import logging
    logger = logging.getLogger('OTBGenerator')
    if not logger.handlers:
        set_OTB_log()
    logger = logging.getLogger('OTBGenerator')
    return logger


def indent(elem, level=0):
    i = "\n" + level * "  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            indent(elem, level + 1)
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i

set_OTB_log()


def get_parameters():
    parameters = {getattr(otbApplication, each): each for each in dir(otbApplication) if 'ParameterType_' in each}
    return parameters


def get_inverted_parameters():
    """
    This function allows mapping otb parameters with processing parameters.
    """
    parameters = {getattr(otbApplication, each): each for each in dir(otbApplication) if 'ParameterType_' in each}

    inverted_parameters = {key: value for value, key in parameters.items()}
    inverted_parameters['ParameterType_Radius'] = 1
    inverted_parameters['ParameterType_RAM'] = 1
    inverted_parameters['ParameterType_ComplexInputImage'] = 9
    inverted_parameters['ParameterType_ComplexOutputImage'] = 13

    inverted_parameters_clone = copy.deepcopy(inverted_parameters)
    inverted_parameters_clone['ParameterType_Empty'] = 'ParameterBoolean'
    inverted_parameters_clone['ParameterType_Int'] = 'ParameterNumber'
    inverted_parameters_clone['ParameterType_Float'] = 'ParameterNumber'
    inverted_parameters_clone['ParameterType_String'] = 'ParameterString'
    inverted_parameters_clone['ParameterType_StringList'] = 'ParameterString'
    inverted_parameters_clone['ParameterType_InputFilename'] = 'ParameterFile'
    inverted_parameters_clone['ParameterType_OutputFilename'] = 'OutputFile'
    inverted_parameters_clone['ParameterType_Directory'] = 'ParameterFile'
    inverted_parameters_clone['ParameterType_Choice'] = 'ParameterSelection'
    inverted_parameters_clone['ParameterType_InputImage'] = 'ParameterRaster'
    inverted_parameters_clone['ParameterType_InputImageList'] = 'ParameterMultipleInput'
    inverted_parameters_clone['ParameterType_InputVectorData'] = 'ParameterVector'
    inverted_parameters_clone['ParameterType_InputVectorDataList'] = 'ParameterMultipleInput'
    inverted_parameters_clone['ParameterType_OutputImage'] = 'OutputRaster'
    inverted_parameters_clone['ParameterType_OutputVectorData'] = 'OutputVector'
    inverted_parameters_clone['ParameterType_Radius'] = 'ParameterNumber'
    inverted_parameters_clone['ParameterType_Group'] = None
    inverted_parameters_clone['ParameterType_ListView'] = 'ParameterSelection'
    inverted_parameters_clone['ParameterType_ComplexInputImage'] = 'ParameterRaster'
    inverted_parameters_clone['ParameterType_ComplexOutputImage'] = 'OutputRaster'
    inverted_parameters_clone['ParameterType_RAM'] = 'ParameterNumber'
    inverted_parameters_clone['ParameterType_InputProcessXML'] = 'ParameterFile'
    inverted_parameters_clone['ParameterType_OutputProcessXML'] = 'ParameterFile'
    inverted_parameters_clone['ParameterType_InputFilenameList'] = 'ParameterMultipleInput'  # 'ParameterString'

    return inverted_parameters_clone


def retrieve_module_name(param):
    """
    returns the file parameter of the given processing parameter
    """
    if param:
        try:
            import processing.core
            dir_p = os.path.dirname(processing.core.__file__)
            if 'Parameter' in param:
                exec("from processing.core.parameters import %s" % param)
                return os.path.join(dir_p, "parameters.py")
            if 'Output' in param:
                exec("from processing.core.outputs import %s" % param)
                return os.path.join(dir_p, "outputs.py")
        except ImportError:
            print "Error parsing ", param
    return None


def get_constructor_parameters_from_filename(py_file, param=""):
    """
    Get all parameters from the constructor of the class param in the given py_file
    """
    import ast
    asto = ast.parse(open(py_file).read())
    # get all class definitions corresponding to param given len(e1) should be 1
    e1 = [each for each in asto.body if isinstance(each, ast.ClassDef) and each.name == param]

    # e1[0].body lists all functions from the class e1[0]
    # e2 is a list of __init__ functions of class e1[0]
    e2 = [each for each in e1[0].body if hasattr(each, "name") and each.name == "__init__"]
    if len(e2) > 0:
        e4 = e2[0].args.args
    else:
        e4 = []
    e5 = [each.id for each in e4]
    return e5


def get_customize_app_functions():
    """
    Get all parameters from the constructor of the class param in the given py_file
    """
    import ast

    py_file = os.path.join(os.path.dirname(__file__), "OTBSpecific_XMLcreation.py")
    asto = ast.parse(open(py_file).read())
    # get all class definitions corresponding to param given len(e1) should be 1
    e1 = [each.name for each in asto.body if isinstance(each, ast.FunctionDef) and each.name.startswith("get")]

    return e1


def get_xml_description_from_application_name(our_app, criteria=None):
    """
    creates an xml containing information about the given our_app
    """
    # creates the application to get the description
    # header
    app_instance = otbApplication.Registry.CreateApplication(our_app)
    root = ET.Element('root')
    app = ET.SubElement(root, 'key')
    app.text = our_app
    executable = ET.SubElement(root, 'exec')
    executable.text = "otbcli_" + our_app
    longname = ET.SubElement(root, 'longname')
    longname.text = app_instance.GetDocName()
    group = ET.SubElement(root, 'group')
    group.text = get_group(app_instance)
    desc = ET.SubElement(root, 'description')
    desc.text = app_instance.GetDescription()

    if not criteria:
        def real_criteria(x):
            return True
    else:
        if not callable(criteria):
            raise Exception("criteria parameter must be a valid python callable")

        real_criteria = criteria

    if len(our_app) == 0:
        raise Exception("App name is empty!")

    # get parameters
    param_keys = [param_key for param_key in app_instance.GetParametersKeys()]
    param_keys = filter(real_criteria, param_keys)

    for param_key in param_keys:
        if not param_key == "inxml" and not param_key == "outxml":
            get_param_descriptor(app.text, app_instance, param_key, root)
    indent(root)
    return root


def get_the_choices(app_instance, our_descriptor, root):
    choices = ET.SubElement(root, 'choices')
    for choice in app_instance.GetChoiceKeys(our_descriptor):
        choice_node = ET.SubElement(choices, 'choice')
        choice_node.text = choice


def get_param_descriptor(appkey, app_instance, our_descriptor, root):
    """
    update the root xml with the data of the parameter given by "our_descriptor"
    """
    logger = get_OTB_log()
    parameters = get_parameters()
    our_type = parameters[app_instance.GetParameterType(our_descriptor)]

    #get the list of mapped parameters (otb/processing)
    inverted_parameters = get_inverted_parameters()

    mapped_parameter = inverted_parameters[our_type]

    file_parameter = retrieve_module_name(mapped_parameter)

    if not file_parameter:
        logger.info("Type %s is not handled yet. (%s, %s)" % (our_type, appkey, our_descriptor))
        return
    the_params = get_constructor_parameters_from_filename(file_parameter, mapped_parameter)

    # special for default values of OpticalCalibration
    if appkey == "OpticalCalibration":
        if "default" in the_params:
            try:
                app_instance.GetParameterAsString(our_descriptor)
            except RuntimeError:
                return

    param = ET.SubElement(root, 'parameter')
    attrs = {'source_parameter_type': parameters[app_instance.GetParameterType(our_descriptor)]}
    if appkey == "Segmentation":
        if parameters[app_instance.GetParameterType(our_descriptor)] == "ParameterType_OutputFilename":
            attrs = {'source_parameter_type': 'ParameterType_OutputVectorData'}
    if appkey == "LSMSVectorization":
        if parameters[app_instance.GetParameterType(our_descriptor)] == "ParameterType_OutputFilename":
            attrs = {'source_parameter_type': 'ParameterType_OutputVectorData'}
    if appkey == "SplitImage":
        if parameters[app_instance.GetParameterType(our_descriptor)] == "ParameterType_OutputImage":
            attrs = {'source_parameter_type': 'ParameterType_OutputFilename'}

    if parameters[app_instance.GetParameterType(our_descriptor)] == "ParameterType_ListView":
        if not appkey == "RadiometricIndices":
            attrs = {'source_parameter_type': 'ParameterType_StringList'}

    param_type = ET.SubElement(param, 'parameter_type', attrib=attrs)

    param_type.text = inverted_parameters[parameters[app_instance.GetParameterType(our_descriptor)]]
    if appkey == "Segmentation":
        if parameters[app_instance.GetParameterType(our_descriptor)] == "ParameterType_OutputFilename":
            param_type.text = "OutputVector"
    if appkey == "LSMSVectorization":
        if parameters[app_instance.GetParameterType(our_descriptor)] == "ParameterType_OutputFilename":
            param_type.text = "OutputVector"
    if appkey == "SplitImage":
        if parameters[app_instance.GetParameterType(our_descriptor)] == "ParameterType_OutputImage":
            param_type.text = "OutputFile"
    if parameters[app_instance.GetParameterType(our_descriptor)] == "ParameterType_ListView":
        if not appkey == "RadiometricIndices":
            param_type.text = "ParameterString"

    # {the_params = get_constructor_parameters_from_filename(file_parameter, mapped_parameter)
    if len(the_params) == 0:
        # if 'Output' in file_parameter:
        if 'output' in file_parameter:
            file_path = os.path.join(os.path.dirname(file_parameter), 'outputs.py')
            the_params = get_constructor_parameters_from_filename(file_path, "Output")
        if 'parameter' in file_parameter:
            file_path = os.path.join(os.path.dirname(file_parameter), 'parameters.py')
            the_params = (file_path)
            the_params = get_constructor_parameters_from_filename(file_path, "Parameter")

    if "self" in the_params:
        #remove self
        the_params.remove("self")  # the_params[1:]
        # to be identical as before !
        if "isSource" in the_params:
            the_params.remove("isSource")
        if "showSublayersDialog" in the_params:
            the_params.remove("showSublayersDialog")
        if "ext" in the_params:
            the_params.remove("ext")
    else:
        raise Exception("Unexpected constructor parameters")

    key = ET.SubElement(param, 'key')
    key.text = our_descriptor
    is_choice_type = False

    for each in the_params:
        if each == "name":
            name = ET.SubElement(param, 'name')

            nametext = app_instance.GetParameterName(our_descriptor)
            if "angle" in nametext:
                name.text = nametext.replace("\xc2\xb0", "deg")
            else:
                name.text = app_instance.GetParameterName(our_descriptor)
            if our_descriptor == "acqui.fluxnormcoeff":
                pass
        elif each == "description":
            desc = ET.SubElement(param, 'description')
            desc.text = app_instance.GetParameterDescription(our_descriptor)
        elif each == "optional":
            optional = ET.SubElement(param, 'optional')
            optional.text = str(not app_instance.IsMandatory(our_descriptor))
        elif each == "default":
            done = False
            reason = []
            try:
                default_value = str(app_instance.GetParameterAsString(our_descriptor))
                done = True
            except:
                reason.append(traceback.format_exc())
            if not done:
                try:
                    default_value = str(app_instance.GetParameterFloat(our_descriptor))
                    done = True
                except:
                    reason.append(traceback.format_exc())
            if not done:
                try:
                    default_value = str(app_instance.GetParameterInt(our_descriptor))
                    done = True
                except:
                    reason.append(traceback.format_exc())

            if done:
                default = ET.SubElement(param, 'default')
                default.text = default_value

                if is_choice_type:
                    the_keys = [a_key for a_key in app_instance.GetChoiceKeys(our_descriptor)]
                    if default_value in the_keys:
                        default.text = str(the_keys.index(default_value))
                    else:
                        default.text = ''
            else:
                logger.debug("A parameter transformation failed, trying default values : for %s, %s, type %s!, conversion message: %s" % (appkey, our_descriptor, parameters[app_instance.GetParameterType(our_descriptor)], str(reason)))
                the_type = parameters[app_instance.GetParameterType(our_descriptor)]
                if the_type == "ParameterType_Int":
                    default_value = "0"
                elif the_type == "ParameterType_Float":
                    default_value = "0.0"
                elif the_type == "ParameterType_Empty":
                    default_value = "True"
                else:
                    raise Exception("Unable to adapt %s, %s, %s, conversion message: %s" % (appkey, our_descriptor, parameters[app_instance.GetParameterType(our_descriptor)], str(reason)))

                default = ET.SubElement(param, 'default')
                default.text = default_value
        else:
            is_choice_type = 'Selection' in param_type.text
            node = ET.SubElement(param, each)
            if is_choice_type:
                get_the_choices(app_instance, our_descriptor, node)


def get_default_parameter_value(app_instance, param):
    parameters = get_parameters()
    try:
        return app_instance.GetParameterAsString(param)
    except:
        the_type = parameters[app_instance.GetParameterType(param)]
        default_value = "0"
        if the_type == "ParameterType_Int":
            default_value = "0"
        elif the_type == "ParameterType_Float":
            default_value = "0.0"
        elif the_type == "ParameterType_Empty":
            default_value = "True"
        return default_value


def escape_html(par):
    if 'Int' in par:
        return '&lt;int32&gt;'
    if 'Float' in par:
        return '&lt;float&gt;'
    if 'Empty' in par:
        return '&lt;boolean&gt;'
    if 'Radius' in par:
        return '&lt;int32&gt;'
    if 'RAM' in par:
        return '&lt;int32&gt;'
    return '&lt;string&gt;'


def is_a_parameter(app_instance, param):
    if app_instance.GetName() == "HaralickTextureExtraction":
        if param.startswith("parameters."):
            return True
    if '.' in param:
        return False
    try:
        app_instance.GetChoiceKeys(param)
        return False
    except:
        return True


def describe_app(app_instance):
    parameters = get_parameters()
    result = []
    with tag('html', result):
        with tag('head', result):
            how = """
<style type="text/css">
dl { border: 3px double #ccc; padding: 0.5em; } dt { float: left; clear: left; text-align: left; font-weight: bold; color: green; } dt:after { content: ":"; } dd { margin: 0 0 0 220px; padding: 0 0 0.5em 0; }
</style>
"""
            result.append(how)
        with tag('body', result):
            with tag('h1', result):
                result.append(app_instance.GetName())
            with tag('h2', result):
                result.append('Brief Description')
            result.append(app_instance.GetDescription())
            with tag('h2', result):
                result.append('Tags')
            result.append(','.join(app_instance.GetDocTags()))
            with tag('h2', result):
                result.append('Long Description')
            result.append(app_instance.GetDocLongDescription())
            with tag('h2', result):
                result.append('Parameters')
            params = app_instance.GetParametersKeys()
            with tag('ul', result):
                for param in params:
                    if is_a_parameter(app_instance, param):
                        with tag('li', result):
                            result.append('<b>%s -%s</b> %s ' % ('[param]', param, escape_html(parameters[app_instance.GetParameterType(param)])))
                            result.append('%s. Mandatory: %s. Default Value: &quot;%s&quot;' % (app_instance.GetParameterDescription(param), str(app_instance.IsMandatory(param)), get_default_parameter_value(app_instance, param)))
                choices_tags = [each for each in params if (not is_a_parameter(app_instance, each)) and '.' not in each]
                for choice in choices_tags:
                    result.append('<b>%s -%s</b> %s %s. Mandatory: %s. Default Value: &quot;%s&quot;' % ('[choice]', choice, app_instance.GetParameterDescription(choice), ','.join(app_instance.GetChoiceKeys(choice)), str(app_instance.IsMandatory(choice)), get_default_parameter_value(app_instance, choice)))
                    choices = app_instance.GetChoiceKeys(choice)

                    with tag('ul', result):
                        for subchoice in choices:
                            with tag('li', result):
                                result.append('<b>%s -%s</b>' % ('[group]', subchoice))
                            with tag('ul', result):
                                param_tags = [each for each in params if '.%s' % subchoice in each]
                                for param_tag in param_tags:
                                    with tag('li', result):
                                        result.append('<b>%s -%s</b> ' % ('[param]', param_tag))
                                        result.append("%s %s. Mandatory: %s. Default Value: &quot;%s&quot;" % (escape_html(parameters[app_instance.GetParameterType(param_tag)]), app_instance.GetParameterDescription(param_tag), str(app_instance.IsMandatory(param_tag)), get_default_parameter_value(app_instance, param_tag)))
            with tag('h2', result):
                result.append('Limitations')
            result.append(app_instance.GetDocLimitations())
            with tag('h2', result):
                result.append('Authors')
            result.append(app_instance.GetDocAuthors())
            with tag('h2', result):
                result.append('See Also')
            result.append(app_instance.GetDocSeeAlso())
            with tag('h2', result):
                result.append('Example of use')
            result.append(app_instance.GetHtmlExample())
    if app_instance.GetName() == "HaralickTextureExtraction":
        index = result.index("<b>[param] -parameters</b> &lt;string&gt; ")
        del result[index + 2]
        del result[index + 1]
        del result[index]
        del result[index - 1]
    return "".join(result)


def get_list_from_node(myet, available_app):
    all_params = []
    for parameter in myet.iter('parameter'):
        rebuild = []
        par_type = parameter.find('parameter_type').text
        key = parameter.find('key').text
        name = parameter.find('name').text
        source_par_type = parameter.find('parameter_type').attrib['source_parameter_type']
        rebuild.append(source_par_type)
        rebuild.append(par_type)
        rebuild.append(key)
        rebuild.append(name)
        for each in parameter[4:]:
            if each.tag not in ["hidden"]:
                if len(each.getchildren()) == 0:
                    if each.tag in ["default"]:
                        if "-" in available_app:
                            available_app = available_app.split("-")[0]
                        app_instance = otbApplication.Registry.CreateApplication(available_app)
                        rebuild.append(get_default_parameter_value(app_instance, key))
                    else:
                        rebuild.append(each.text)
                else:
                    rebuild.append([item.text for item in each.iter('choice')])
        all_params.append(rebuild)
    return all_params


def adapt_list_to_string(c_list):
    a_list = c_list[1:]
    if a_list[0] in ["ParameterVector", "ParameterMultipleInput"]:
        if c_list[0] == "ParameterType_InputImageList":
            a_list[3] = 3
        else:
            a_list[3] = -1

    if a_list[0] in ["ParameterRaster", "ParameterFile", "ParameterMultipleInput", "OutputRaster", "OutputFile"]:
        if "Output" in a_list[0]:
            a_list.append("/tmp/processing/output.tif")
        else:
            import os
            a_list.append(os.path.join(os.path.abspath(os.curdir), "helper/QB_Toulouse_Ortho_PAN.tif"))

    if a_list[0] in ["ParameterSelection"]:
        pass

    a_list[1] = "-%s" % a_list[1]

    def mystr(par):
        if isinstance(par, list):
            return ";".join(par)
        return str(par)

    if a_list[-1] is None:
        return ""

    b_list = map(mystr, a_list)
    b_list = [b_list[1], b_list[-1]]
    res = " ".join(b_list)
    return res


def get_automatic_ut_from_xml_description(the_root):
    dom_model = the_root

    try:
        appkey = dom_model.find('key').text
        cliName = dom_model.find('exec').text

        if not cliName.startswith("otbcli_"):
            raise Exception('Wrong client executable')

        rebu = get_list_from_node(dom_model, appkey)
        the_result = map(adapt_list_to_string, rebu)
        ut_command = cliName + " " + " ".join(the_result)
        return ut_command
    except Exception:
        ET.dump(dom_model)
        raise


def list_reader(file_name, version):
    tree = ET.parse(file_name)
    root = tree.getroot()
    nodes = [each.text for each in root.findall("./version[@id='%s']/app_name" % version)]
    return nodes


def get_otb_version():
    #TODO Find a way to retrieve installed otb version, force exception and parse otb-X.XX.X ?
    # return "3.18"
    return "5.0"


def get_white_list():
    nodes = list_reader("white_list.xml", get_otb_version())
    return nodes


def get_black_list():
    nodes = list_reader("black_list.xml", get_otb_version())
    return nodes


def create_xml_descriptors():
    import os
    if not os.path.exists("description"):
        os.mkdir("description")
    if not os.path.exists("html"):
        os.mkdir("html")

    logger = get_OTB_log()

    white_list = get_white_list()
    black_list = get_black_list()
    custom_apps_available = get_customize_app_functions()

    for available_app in otbApplication.Registry.GetAvailableApplications():
        # try:
        if 'get%s' % available_app in custom_apps_available:
            if available_app in white_list and available_app not in black_list:
                the_list = []
                the_root = get_xml_description_from_application_name(available_app)
                function_to_call = "the_list = OTBSpecific_XMLcreation.get%s(available_app,the_root)" % available_app
                exec(function_to_call)
                # the_list = locals()['get%s' % available_app](available_app, the_root)
                if the_list:
                    for each_dom in the_list:
                        try:
                            ut_command = get_automatic_ut_from_xml_description(each_dom)  # NOQA
                        except:
                            logger.error("Unit test for command %s must be fixed: %s" % (available_app, traceback.format_exc()))
            else:
                logger.warning("%s is not in white list." % available_app)

        else:
            if available_app in white_list and available_app not in black_list:
                logger.warning("There is no adaptor for %s, check white list and versions" % available_app)
                # TODO Remove this default code when all apps are tested...
                fh = open("description/%s.xml" % available_app, "w")
                the_root = get_xml_description_from_application_name(available_app)
                ET.ElementTree(the_root).write(fh)
                fh.close()
                try:
                    get_automatic_ut_from_xml_description(the_root)
                except:
                    logger.error("Unit test for command %s must be fixed: %s" % (available_app, traceback.format_exc()))

        # except Exception, e:
        #    logger.error(traceback.format_exc())


def create_html_description():
    logger = get_OTB_log()

    if not os.path.exists("description/doc"):
        os.mkdir("description/doc")

    for available_app in otbApplication.Registry.GetAvailableApplications():
        try:
            fh = open("description/doc/%s.html" % available_app, "w")
            app_instance = otbApplication.Registry.CreateApplication(available_app)
            app_instance.UpdateParameters()
            ct = describe_app(app_instance)
            fh.write(ct)
            fh.close()
        except Exception:
            logger.error(traceback.format_exc())

    sub_algo = [each for each in os.listdir("description") if "-" in each and ".xml" in each]
    for key in sub_algo:
        shutil.copy("description/doc/%s" % key.split("-")[0] + ".html", "description/doc/%s" % key.split(".")[0] + ".html")

if __name__ == "__main__":
    # Prepare the environment
    from qgis.core import QgsApplication
    from qgis.PyQt.QtWidgets import QApplication
    app = QApplication([])
    QgsApplication.setPrefixPath("/usr", True)
    QgsApplication.initQgis()
    # Prepare processing framework
    from processing.core.Processing import Processing
    Processing.initialize()

#    import OTBSpecific_XMLcreation
#     try:
#         import processing
#     except ImportError, e:
#         raise Exception("Processing must be installed and available in PYTHONPATH")

    try:
        import otbApplication
    except ImportError as e:
        raise Exception("OTB python plugins must be installed and available in PYTHONPATH")

    create_xml_descriptors()
    create_html_description()

    # Exit applications
    QgsApplication.exitQgis()
    QApplication.exit()
