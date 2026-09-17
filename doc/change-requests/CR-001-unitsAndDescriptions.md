# CR-001: Add unit and description texts

Status: in progress

## Requirements

Units and descriptions of process variables (i.e. `TransferElement`s of an application or connected device), should be automatically available in the DOOCS control system.

We need additional XML configuration syntax to override units or descriptions at the process variable level.
Furthermore, we must maintain support for the current method of setting unit and/or description texts from the control system.
(which may be done either by pre-populating .doocs config files, or via doocs-set call over the network).
So three cases need to be distinguished, as data source
(a) unit/description inherited statically from `TransferElement`
(b) unit/description set in XML config, statically
(c) unit/description set dynamically via control system, by user

The default behavior should be (a), where it makes sense. For types where no meaningful description can be derived from the
involved process variables (D_iiii/D_ifff), default behavior should be (c).
We must support overriding default behavior on property, location or global level, as behavior (c).

## Specifications

The XML syntax should look like this (excerpt of `<serverName>-DoocsVariableConfig.xml`):

```xml
<device_server>
<!-- this is already global default: -->
<ignore_description_from_app>false</ignore_description_from_app>
    <location name="LocA">
        <property source="A1" />
        <property source="C2">
            <description>the overwritten description</description>
        </property>
    </location>
    <location name="LocB">
        <D_spectrum source="B1">
          <startSource>B1start</startSource>
          <incrementSource>B1inc</incrementSource>
        </D_spectrum>
        <property source="B2">
            <description>the overwritten description</description>
        </property>
        <property source="B3">
            <ignore_description_from_app>true</ignore_description_from_app>
        </property>
    </location>
    <location name="LocC">
        <ignore_description_from_app>true</ignore_description_from_app>
        <property source="C1">
        </property>
        <D_xy source="C2">
            <description>the overwritten description</description>
            <unit axis="x">xUnit</unit>
            <unit axis="y">yUnit</unit>
        </D_xy>
    </location>
</device_server>
```

So in this example, locations `LocA` and `LocB` inherit descriptions from process variables,
while `LocC` takes them from the control system.
But anywhere, if `<description>` is specified for a property, the enclosed text is used and overrides all other sources.
 `<ignore_description_from_app>` also controls the default source of the unit texts.
If not ignored, and the XML config of a `D_spectrum` property has `<startSource>` or `<incrementSource>`, we should derive the x-unit from there.
If both exist, unit text from `<incrementSource>` wins.
In the example above, `B3` uses only manually set or .conf-persisted unit and description texts.
 
### Analysis: DOOCS ways of setting unit description texts
 * `D_spectrum`: description goes into `<name>.COMMENT` property, type STRING,
    units x/y go into `<name>.XEGU` and `<name>.EGU` properties, type A_USTR which is (int, float, float, string)
   
 * `D_xy`: we already have xml syntax `<description>` and `<unit axis="x">`, `<unit axis="y">`.
   Similar to `D_spectrum` on DOOCS side, properties `<name>.COMMENT`, `<name>.XEGU`, and `<name>.EGU` are used.
   
 * Scalars `D_value<valueType>` valueType=(bool|int|float|double|str)
   for scalar DOOCS properties `<name>.DESC` and `<name>.EGU` are used.
   These are created by the DOOCS auto historizer, see `D_value<valueType>::get_histPointer`. 
   But be careful, sometimes we create scalars without a history!
 
 * Arrays `D_array<valueType>` valueType=(bool|int|long|float|double)
   Not sure whether DOOCS unit and description texts exist as separate properties.
   But it should be possible to add a short description text by adding it space-separated to the DOOCS property name.
   
 * Combined types `D_iiii`, `D_ifff`
   These support `D_hist` interface, and by that descriptions and engineering units.
   Do not fill in the description and unit texts from the process variables since it's not clear how
   the four inputs should be combined. So here, `<ignore_description_from_app>` is always true, but a custom `<description>` may be present.
   Also `<unit>` is a valid flag, `axis="x"` optional. The user should enter a unit description for all fields. 

 * Images `D_imagec`
   This has neither `.DESC` nor `.EGU` property. Images do have a comment, but the DOOCS api to retreive it back seems unpratical.
   We should create `.DESC` manually, but do not create `.EGU`, `.XEGU`.

Design decisions: 
*  Whenever classes do not provide unit/description either directly or via historizing interface, we should create
   the DOOCS sub-properties `<name>.EGU` and `<name>.DESC`. In particular that holds for scalars without history, and arrays.


### DOOCS server api functions, to set descriptions

| Property class | Description sub-property | Unit mechanism | Setter / getter API |
|---|---|---|---|
| `D_spectrum` | `.COMMENT` (D_string) | `.XEGU` (x), `.EGU` (y) — both `D_plotinfo` | `set_descr_value(desc)`, `description(desc)` (only-if-empty), `set_plot_x_unit(u)`, `set_plot_y_unit(u)`, `xegu(linlog,start,stop,u)`, `egu(...)`; getters `description()`, `plot_x_unit()`, `plot_y_unit()` |
| `D_xy` | `.COMMENT` (D_string) | `.XEGU`, `.EGU` (D_plotinfo) | `set_descr_value(desc)`, `description(desc)`, `set_plot_x_unit(u)`, `set_plot_y_unit(u)`, `xegu(...)`, `egu(...)` |
| `D_imagec` / `D_image` | comment | — (no unit) | `set_descr_value(com)`/`descr_value()`; `D_image` also `set_img_comment` |
| Scalar `D_value<T>`, `D_iiii`, `D_ifff` | **via historizing `D_hist`** | **via `D_hist` `.EGU`** | see below |
| `D_array<T>` (arrays) | **none** (no D_hist) | **none** | no dedicated API |


## Implementation notes and alternatives considered

* If possible, the implementation should be done in `PropertyBase`.
  If DOOCS API for setting description and unit differs per class, it needs to be done in the specific class.
  Add non-virtual setters `setUnit()`, `setDescription()`, which just store the meta-information, and a
  virtual `PropertyBase` method for applying them appropriately for the respective DOOCS type.

* Before adding any functionality, we should refactor `VariableMapper.cc` so that there is only a single definition of `struct Axis`,
  used by `XyDescription` and `SpectrumDescription`.
  That one will later be used from other places as well. We keep all fields.
  In particular, scalar and arrays classes should parse full definition of `struct Axis`, and apply either via historizing interface `D_hist`,
  or self-created `.EGU` sub-property. We should unify parsing, for all types.
 
* If possible, apply unit/description on DOOCS types only once. It's not so clear whether we can find a good lifecycle stage where everything is set up,
  independent of the DOOCS type. We must be careful to set unit and description texts only after the DOOCS .conf was loaded, otherwise settings would
  be overwritten from there even when we don't want that.
  We should set unit/description in overwritten `auto_init()`. I checked that it is always called, independent of whether location already existed in .conf file.
  Avoid applying them already from `DoocsPVFactory`.
  
* After setting known unit/description from either our XML or the process variable, we must make that sub-property read-only.
  
* Existing branch `feat-EGU` logs a warning when there is a difference between description text persisted in DOOCS .conf and value provided 
  from DOOCSAdapter (from xml of process variable). We should not warn at all - since descriptions are persisted in the .conf the warning would re-appear
  every time a description is changed, which is too often.

* We should talk to DOOCS team about possible plans to unify units and descriptions for all types - also arrays and scalars without history.

* Should we set the description by the old-fashioned way, by appending it to the property name?
  It seems that would be a static, unchangeable description, and length limited to the leftover of 80 chars, subtracting name.
  If we want that, we must already set descriptions in the `DoocsPVFactory`!
  Decision: For now, do not use old-fashioned description, often available string is much too short. 

* When falling back to self-created `.DESC` properties, should we use `D_string` or `D_text`?
  We use `D_string`, for consistency with already present `.DESC`, but unfurtunately that's limited to 80 characters.
   
### Test plan

* We should create a new unit test (don't want to complicate existing tests further)
* The test config should include one configuration per type (scalar, array, `D_spectrum`, `D_xy`, `D_iii`, `D_ifff`, `D_image`)
* test automatically filled in unit and description texts as well as overwritten ones

### Further work items

adapt XML scheme
 
